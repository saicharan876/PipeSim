# PipeSim: A 5-Stage CPU Pipeline Simulator

This project simulates a basic **5-stage pipelined processor** capable of executing a simplified instruction set. It reads instructions and data from external files, simulates instruction execution with hazards and stalls, and produces performance statistics and updated memory states.

---

##  Pipeline Stages

The processor follows a classic 5-stage pipeline model:

1. **IF (Instruction Fetch)**  
2. **ID (Instruction Decode & Hazard Check)**  
3. **EX (Execute / ALU Operations)**  
4. **MEM (Memory Access)**  
5. **WB (Write Back)**  

It accounts for:
- **Data Hazards** (RAW)
- **Control Hazards** (Branches, Jumps)
- **Pipeline Stalls**

---

##  File Structure

| File Name        | Description                              |
|------------------|------------------------------------------|
| `coa.cpp`        | Main C++ simulator implementation         |
| `ICache.txt`     | Input instruction cache (256 lines, hex) |
| `DCache.txt`     | Initial data memory (256 lines, hex)     |
| `RF.txt`         | Initial register file (16 lines, hex)    |
| `Output.txt`     | Output performance statistics            |
| `ODCache.txt`    | Output data cache after execution        |

---

##  Features

- Simulates instruction-by-instruction execution
- Detects and handles:
  - **Data stalls (RAW hazards)**
  - **Control stalls (branches/jumps)**
- Tracks execution stats:
  - Total instructions
  - Instruction types (arithmetic, logical, memory, control, halt)
  - Clock cycles, stalls
  - CPI (Cycles Per Instruction)
- Outputs final data memory state

---

##  How to Run

### Prerequisites
- C++ compiler (e.g. `g++`)
- Input files: `ICache.txt`, `DCache.txt`, `RF.txt`

### Compile and Run

```bash
g++ PipeSim.cpp -o PipeSim
./PipeSim
