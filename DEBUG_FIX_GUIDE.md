## 🔧 **UART Application Debug Fix Guide**

You're encountering common Zynq debugging issues. Here's the step-by-step solution:

### **Error Analysis:**
- `Cannot reset APU` - ARM Processing Unit reset failure
- `Memory write error at 0x100000` - DDR access issue during initialization
- `Cannot flush CPU cache` - Cache operation failure

### **Root Causes:**
1. **Hardware not properly initialized** before debugging
2. **FSBL not running** to initialize DDR and clocks
3. **Debug configuration** pointing to wrong files
4. **Platform configuration** mismatch

---

## 🛠️ **Step-by-Step Fix:**

### **Step 1: Build Application First**
```bash
# In Vitis, right-click on app_component
# Select: "Build Project"
# Wait for build to complete
```

### **Step 2: Correct Debug Launch Sequence**
I've updated your `launch.json` with these critical fixes:
- ✅ `"resetAPU": true` - Force APU reset
- ✅ `"stopAtEntry": true` - Stop at main() for debugging
- ✅ Fixed FSBL path to correct location

### **Step 3: Platform Regeneration** (if needed)
```bash
# If issues persist, regenerate platform:
# 1. Right-click "ARTY" platform
# 2. Select "Clean Platform"
# 3. Select "Build Platform" 
```

### **Step 4: Debugging Sequence**
1. **Program FPGA first** (bitstream)
2. **Run FSBL** to initialize hardware
3. **Then debug application**

---

## 🎯 **Alternative Debug Methods:**

### **Method 1: Boot from SD Card (Recommended)**
Create BOOT.BIN with:
```
FSBL + Bitstream + Application
```

### **Method 2: Two-Stage Debug**
1. First run FSBL standalone
2. Then attach debugger to application

### **Method 3: Hardware Manager**
Use Vivado Hardware Manager to:
1. Program bitstream
2. Initialize PS via TCL script
3. Then debug in Vitis

---

## 🔍 **Verification Steps:**

### **Check Hardware Connection:**
```tcl
# In Vitis TCL Console:
connect -url tcp:localhost:3121
targets -set -nocase -filter {name =~ "*APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent*"}
fpga -file "bitstream/artyz7_20_platform.bit"
targets -set -nocase -filter {name =~ "*A9*#0"}
loadhw -hw "hw/artyz7_20_platform.xsa" -mem-ranges [list {0x40000000 0x7FFFFFFF}]
configparams force-mem-access 1
source "psinit/ps7_init.tcl"
ps7_init
ps7_post_config
```

### **Manual Memory Test:**
```tcl
# Test DDR access:
mwr 0x100000 0x12345678
mrd 0x100000
# Should return: 0x100000: 12345678
```

---

## 🎛️ **Debug Configuration Options:**

Try these alternative launch configurations:

### **Option A: No FSBL (Direct Boot)**
```json
"usingFSBL": {
  "initWithFSBL": false
},
"usingPs7Init": {
  "runPs7Init": true,
  "runPs7PostInit": true
}
```

### **Option B: Reset Everything**
```json
"resetSystem": true,
"resetAPU": true,
"resetProcessor": true
```

---

## ⚡ **Quick Fix Command Sequence:**

1. **Close debug session** if running
2. **Clean and rebuild** application
3. **Power cycle** the ARTY board
4. **Start fresh debug session**

The updated launch.json should resolve most issues. If problems persist, use the TCL script method to manually initialize the hardware first.