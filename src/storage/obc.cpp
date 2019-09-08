// Copyright (c) 2019 EPI-ONE Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "obc.h"

OrphanBlocksContainer::~OrphanBlocksContainer() {
    block_dep_map_.clear();
    lose_ends_.clear();
}

std::size_t OrphanBlocksContainer::Size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return block_dep_map_.size();
}

size_t OrphanBlocksContainer::DependencySize() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return lose_ends_.size();
}

bool OrphanBlocksContainer::IsEmpty() const {
    return this->Size() == 0;
}

bool OrphanBlocksContainer::Contains(const uint256& hash) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return block_dep_map_.find(hash) != block_dep_map_.end();
}

void OrphanBlocksContainer::AddBlock(const ConstBlockPtr& block, uint8_t missing_mask) {
    if (missing_mask == 0) {
        return;
    }

    // insert new dependency into block_dep_map_
    std::unique_lock<std::shared_mutex> writer(mutex_);

    // construct new dependency for the new block
    obc_dep_ptr dep = std::make_shared<obc_dep>();
    dep->block      = block;

    // insert new dependency into block_dep_map_
    std::unordered_set<uint256> unique_missing_hashes;

    auto common_insert = [&](uint256&& hash) {
        auto it = block_dep_map_.find(hash);
        unique_missing_hashes.insert(hash);

        if (it == block_dep_map_.end()) {
            // if the dependency is not in this OBC then the dep is a lose end
            lose_ends_[hash].insert(dep);
        } else {
            // if the dependency is in the OBC the dep is linked to the dependency dep
            it->second->deps.push_back(dep);
        }
    };

    // insert new dependency into block_dep_map_
    block_dep_map_.insert_or_assign(block->GetHash(), dep);

    if (missing_mask & M_MISSING) {
        common_insert(block->GetMilestoneHash());
    }

    if (missing_mask & T_MISSING) {
        common_insert(block->GetTipHash());
    }

    if (missing_mask & P_MISSING) {
        common_insert(block->GetPrevHash());
    }

    dep->ndeps = unique_missing_hashes.size();
}

std::optional<std::vector<ConstBlockPtr>> OrphanBlocksContainer::SubmitHash(const uint256& hash) {
    std::unique_lock<std::shared_mutex> writer(mutex_);
    auto range = lose_ends_.find(hash);

    // if no lose ends can be tied using this hash return
    if (range == lose_ends_.end()) {
        return {};
    }

    std::vector<obc_dep_ptr> stack;
    std::vector<ConstBlockPtr> result;

    // for all deps that have the given hash as a parent/dependency
    for (auto& n : range->second) {
        // push it onto the stack as it might be used later on
        stack.push_back(n);
    }

    /* the addition of the given hash
     * ties lose ends therefore the
     * corresponding dependencies have
     * to be removed from lose_ends_ */
    lose_ends_.erase(range);
    writer.unlock();

    obc_dep_ptr cursor;
    while (!stack.empty()) {
        // pop dependency from the stack
        cursor = stack.back();
        stack.pop_back();

        // decrement the number of missing dependencies
        cursor->ndeps--;
        if (cursor->ndeps > 0) {
            continue;
        }

        /* by now we know that all blocks are available
         * meaning that the block is return as it is not
         * an orphan anymore */
        result.push_back(cursor->block);

        /* this also means that we have to remove the
         * dependency from the block_dep_map_ */
        writer.lock();
        block_dep_map_.erase(cursor->block->GetHash());
        writer.unlock();

        /* further we push all dependencies that dependend
         * on this block onto the stack */
        for (const obc_dep_ptr& dep : cursor->deps) {
            stack.push_back(dep);
        }
    }

    return result;
}