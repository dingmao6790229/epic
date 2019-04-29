#ifndef __SRC_TRANSACTION_H__
#define __SRC_TRANSACTION_H__

#include <cassert>
#include <limits>
#include <sstream>
#include <unordered_set>

#include "hash.h"
#include "params.h"
#include "script.h"
#include "tinyformat.h"

static const uint32_t UNCONNECTED = UINT_LEAST32_MAX;

class Block;
class Transaction;

class TxOutPoint {
public:
    uint256 bHash;
    uint32_t index;

    TxOutPoint() : index(UNCONNECTED) {}

    // TODO: search for the pointer of Block in Cat
    TxOutPoint(const uint256 fromBlock, const uint32_t index) : bHash(fromBlock), index(index) {}

    friend bool operator==(const TxOutPoint& out1, const TxOutPoint& out2) {
        return out1.index == out2.index && out1.bHash == out2.bHash;
    }

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(bHash);
        READWRITE(index);
    }
};

/** Key hasher for unordered_set */
template <>
struct std::hash<TxOutPoint> {
    size_t operator()(const TxOutPoint& x) const {
        return std::hash<uint256>()(x.bHash) + (size_t) index;
    }
};

class TxInput {
public:
    TxOutPoint outpoint;
    Script scriptSig; // MAX: actual listing plus program must be here not in Output.

    TxInput() = default;

    explicit TxInput(const TxOutPoint& outpoint, const Script& scriptSig = Script());
    // TASM How is scriptsig inhabited in the constructor? Do we get it from the message, right?
    // MAX: we get in over the network
    // QING: Max is correct

    TxInput(const uint256& fromBlock, const uint32_t index, const Script& scriptSig = Script());

    TxInput(const Script& script);

    bool IsRegistration() const;

    bool IsFirstRegistration() const;

    void SetParent(const Transaction* const tx);

    const Transaction* GetParentTx();

    friend bool operator==(const TxInput& a, const TxInput& b) {
        return (a.outpoint == b.outpoint) && (a.scriptSig.bytes == b.scriptSig.bytes);
    }

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(outpoint);
        READWRITE(scriptSig);
    }

private:
    const Transaction* parentTx_;
};

class TxOutput {
public:
    // TODO: implement Coin class
    Coin value;
    Script scriptPubKey;
    // TASM push pubkey to vstream and create a listing with a verify op.
    // QING: scriptPubKey is the serialized listing#data

    TxOutput();

    TxOutput(const Coin& value, const Script& scriptPubKey);

    void SetParent(const Transaction* const tx);

    friend bool operator==(const TxOutput& a, const TxOutput& b) {
        return (a.value == b.value) && (a.scriptPubKey.bytes == b.scriptPubKey.bytes);
    }

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(value);
        READWRITE(scriptPubKey);
    }

private:
    const Transaction* parentTx_;
};

class Transaction {
public:
    enum Validity : uint8_t {
        UNKNOWN = 0,
        VALID,
        INVALID,
    };

    Transaction();

    explicit Transaction(const Transaction& tx);

    Transaction& AddInput(TxInput&& input);

    Transaction& AddOutput(TxOutput&& output);

    void FinalizeHash();

    const TxInput& GetInput(size_t index) const;

    const TxOutput& GetOutput(size_t index) const;

    const std::vector<TxInput>& GetInputs() const;

    const std::vector<TxOutput>& GetOutputs() const;

    const uint256& GetHash() const;

    bool IsRegistration() const;

    bool IsFirstRegistration() const;

    bool Verify() const;

    void Validate();

    void Invalidate();

    void SetStatus(Validity&& status);

    Validity GetStatus() const;

    void SetParent(const Block* const blk);

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(inputs);
        READWRITE(outputs);
    }

    friend bool operator==(const Transaction& a, const Transaction& b) {
        return a.GetHash() == b.GetHash();
    }

private:
    std::vector<TxInput> inputs;
    std::vector<TxOutput> outputs;
    // TASM should we merge the listings about to the member of transaction? MAX:let's do it this way
    // TASM Where do you want to initialize the interpreter? MAX: DAG MANAGER
    // QING: I think a better way could be: Tx contains the Script which is basically just the serialized listing. Tasm interpreter takes the Script, deserializes it into listing, and do whatever logic required by the listing#program. In this way Transaction won't care about how Tasm works and it's job is only to carry the information.

    uint256 hash_;
    Coin fee_;
    Validity status_;

    const Block* parentBlock_;
};

namespace std {
string to_string(const TxOutPoint& outpoint);
string to_string(const TxInput& input);
string to_string(const TxOutput& output);
string to_string(Transaction& tx);
} // namespace std

#endif //__SRC_TRANSACTION_H__
