
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

using u32 = uint32_t;
using s32 = int32_t;
using u8 = uint8_t;

const u32 TEXT_BASE = 0x00400000;
const size_t INSTRUCTION_COUNT = 42;
const size_t TEXT_SIZE = INSTRUCTION_COUNT * 4;

struct Instruction {
    u32 address;
    u32 word;
};

string regName(unsigned r) {
    static const string regs[32] = {
        "$zero", "$at", "$v0", "$v1",
        "$a0", "$a1", "$a2", "$a3",
        "$t0", "$t1", "$t2", "$t3",
        "$t4", "$t5", "$t6", "$t7",
        "$s0", "$s1", "$s2", "$s3",
        "$s4", "$s5", "$s6", "$s7",
        "$t8", "$t9", "$k0", "$k1",
        "$gp", "$sp", "$fp", "$ra"
    };
    return regs[r & 31];
}

string labelName(u32 address) {
    stringstream ss;
    ss << "L_" << uppercase << hex << setw(8)
       << setfill('0') << address;
    return ss.str();
}

string hexWord(u32 word) {
    stringstream ss;
    ss << "0x" << uppercase << hex << setw(8)
       << setfill('0') << word;
    return ss.str();
}

s32 signExtend16(u32 word) {
    return static_cast<int16_t>(word & 0xffff);
}

u32 getBranchTarget(u32 pc, u32 word) {
    return pc + 4 +
           static_cast<u32>(signExtend16(word) * 4);
}

u32 getJumpTarget(u32 pc, u32 word) {
    return ((pc + 4) & 0xF0000000u) |
           ((word & 0x03FFFFFFu) << 2);
}

string decode(u32 pc, u32 w) {
    unsigned op = (w >> 26) & 0x3F;
    unsigned rs = (w >> 21) & 31;
    unsigned rt = (w >> 16) & 31;
    unsigned rd = (w >> 11) & 31;
    unsigned sh = (w >> 6) & 31;
    unsigned fn = w & 63;
    s32 imm = signExtend16(w);

    stringstream out;

    if (op == 0) {
        switch (fn) {
            case 0:
                out << "sll " << regName(rd) << ", "
                    << regName(rt) << ", " << sh;
                break;
            case 8:
                out << "jr " << regName(rs);
                break;
            case 12:
                out << "syscall";
                break;
            case 16:
                out << "mfhi " << regName(rd);
                break;
            case 18:
                out << "mflo " << regName(rd);
                break;
            case 24:
                out << "mult " << regName(rs) << ", "
                    << regName(rt);
                break;
            case 26:
                out << "div " << regName(rs) << ", "
                    << regName(rt);
                break;
            case 32:
                out << "add " << regName(rd) << ", "
                    << regName(rs) << ", " << regName(rt);
                break;
            case 33:
                out << "addu " << regName(rd) << ", "
                    << regName(rs) << ", " << regName(rt);
                break;
            case 34:
                out << "sub " << regName(rd) << ", "
                    << regName(rs) << ", " << regName(rt);
                break;
            case 36:
                out << "and " << regName(rd) << ", "
                    << regName(rs) << ", " << regName(rt);
                break;
            case 37:
                out << "or " << regName(rd) << ", "
                    << regName(rs) << ", " << regName(rt);
                break;
            case 42:
                out << "slt " << regName(rd) << ", "
                    << regName(rs) << ", " << regName(rt);
                break;
            default:
                out << ".word " << hexWord(w);
        }
        return out.str();
    }

    switch (op) {
        case 2:
        case 3:
            out << (op == 2 ? "j " : "jal ")
                << labelName(getJumpTarget(pc, w));
            break;

        case 4:
        case 5:
            out << (op == 4 ? "beq " : "bne ")
                << regName(rs) << ", " << regName(rt)
                << ", " << labelName(getBranchTarget(pc, w));
            break;

        case 8:
            out << "addi " << regName(rt) << ", "
                << regName(rs) << ", " << imm;
            break;

        case 9:
            out << "addiu " << regName(rt) << ", "
                << regName(rs) << ", " << imm;
            break;

        case 10:
            out << "slti " << regName(rt) << ", "
                << regName(rs) << ", " << imm;
            break;

        case 12:
            out << "andi " << regName(rt) << ", "
                << regName(rs) << ", " << (w & 0xffff);
            break;

        case 13:
            out << "ori " << regName(rt) << ", "
                << regName(rs) << ", " << (w & 0xffff);
            break;

        case 15:
            out << "lui " << regName(rt) << ", "
                << (w & 0xffff);
            break;

        default:
            out << ".word " << hexWord(w);
    }

    return out.str();
}

int main() {
    const string inputFile = "fizzbuzz.mhc-bin";
    const string asmFile = "reconstructed.asm";
    const string reportFile = "disassembly.txt";

    ifstream in(inputFile, ios::binary);

    if (!in) {
        cerr << "Cannot open " << inputFile << '\n';
        cerr << "Check the CLion working directory.\n";
        return 1;
    }

    vector<u8> bytes;
    char c;

    while (in.get(c)) {
        bytes.push_back(
            static_cast<u8>(static_cast<unsigned char>(c)));
    }

    if (bytes.size() < TEXT_SIZE) {
        cerr << "File is too small for 42 instructions.\n";
        cerr << "Expected at least " << TEXT_SIZE
             << " bytes; got " << bytes.size() << ".\n";
        return 1;
    }

    vector<Instruction> instructions;
    set<u32> labels;

    // Read only the 42 instructions, not the strings in .data.
    for (size_t i = 0; i < TEXT_SIZE; i += 4) {
        u32 word =
            (u32(bytes[i]) << 24) |
            (u32(bytes[i + 1]) << 16) |
            (u32(bytes[i + 2]) << 8) |
            u32(bytes[i + 3]);

        instructions.push_back({
            TEXT_BASE + static_cast<u32>(i), word
        });
    }

    // Find all branch and jump targets.
    for (const auto& ins : instructions) {
        unsigned op = ins.word >> 26;

        if (op == 4 || op == 5) {
            labels.insert(getBranchTarget(ins.address, ins.word));
        } else if (op == 2 || op == 3) {
            labels.insert(getJumpTarget(ins.address, ins.word));
        }
    }

    ofstream asmOut(asmFile);
    ofstream reportOut(reportFile);

    if (!asmOut || !reportOut) {
        cerr << "Cannot create output files.\n";
        return 1;
    }

    asmOut << "# Reconstructed FizzBuzz MIPS program\n";
    asmOut << ".text\n";
    asmOut << ".globl main\n";
    asmOut << "main:\n";

    reportOut << "ADDRESS    MACHINE WORD  INSTRUCTION\n";

    for (const auto& ins : instructions) {
        if (labels.count(ins.address)) {
            asmOut << labelName(ins.address) << ":\n";
        }

        string instruction = decode(ins.address, ins.word);
        asmOut << "    " << instruction << '\n';

        reportOut << hexWord(ins.address) << "  "
                  << hexWord(ins.word) << "  "
                  << instruction << '\n';
    }

    cout << "Disassembly successful.\n";
    cout << "Input size: " << bytes.size() << " bytes\n";
    cout << "Instructions decoded: " << instructions.size() << '\n';
    cout << "Assembly: " << asmFile << '\n';
    cout << "Report: " << reportFile << '\n';

    return 0;
}
