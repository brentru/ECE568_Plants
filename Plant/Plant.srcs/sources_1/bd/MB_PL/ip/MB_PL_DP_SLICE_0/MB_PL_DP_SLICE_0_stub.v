// Copyright 1986-2017 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2017.4 (win64) Build 2086221 Fri Dec 15 20:55:39 MST 2017
// Date        : Mon Apr  9 23:05:27 2018
// Host        : DESKTOP-5J8LVLP running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top MB_PL_DP_SLICE_0 -prefix
//               MB_PL_DP_SLICE_0_ MB_PL_DP_SLICE_0_stub.v
// Design      : MB_PL_DP_SLICE_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a100tcsg324-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "xlslice_v1_0_1_xlslice,Vivado 2017.4" *)
module MB_PL_DP_SLICE_0(Din, Dout)
/* synthesis syn_black_box black_box_pad_pin="Din[19:0],Dout[7:0]" */;
  input [19:0]Din;
  output [7:0]Dout;
endmodule
