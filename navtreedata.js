/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "OMSim", "index.html", [
    [ "OMSim Geant4 Framework", "index.html", [
      [ "Installation", "index.html#autotoc_md58", [
        [ "Installing Geant4", "index.html#autotoc_md59", null ],
        [ "Installing Dependencies", "index.html#autotoc_md60", null ],
        [ "Installing ROOT", "index.html#autotoc_md61", null ],
        [ "Compiling OMSim", "index.html#autotoc_md62", null ]
      ] ],
      [ "Available studies", "index.html#autotoc_md63", [
        [ "Customising Compilation", "index.html#autotoc_md64", null ]
      ] ]
    ] ],
    [ "Framework functionality", "md_extra_doc_0_common.html", [
      [ "Materials and User Data", "md_extra_doc_0_common.html#autotoc_md2", [
        [ "Material Handling", "md_extra_doc_0_common.html#autotoc_md3", null ],
        [ "Optical Surfaces", "md_extra_doc_0_common.html#autotoc_md4", null ],
        [ "Special Material Types", "md_extra_doc_0_common.html#autotoc_md5", null ],
        [ "Geometry Data", "md_extra_doc_0_common.html#autotoc_md6", null ],
        [ "Adding New Data", "md_extra_doc_0_common.html#autotoc_md7", null ]
      ] ],
      [ "Geometry construction", "md_extra_doc_0_common.html#autotoc_md9", null ],
      [ "Making PMTs and OMs sensitive", "md_extra_doc_0_common.html#autotoc_md11", null ],
      [ "Storing hits and PMT response", "md_extra_doc_0_common.html#autotoc_md13", [
        [ "PMTs Charge, transit time and detection probability", "md_extra_doc_0_common.html#autotoc_md14", null ],
        [ "Hit storage", "md_extra_doc_0_common.html#autotoc_md15", null ]
      ] ],
      [ "Making other volumes sensitive to photons", "md_extra_doc_0_common.html#autotoc_md17", null ]
    ] ],
    [ "Multi-threading mode", "md_extra_doc_1_multithreading.html", [
      [ "Introduction", "md_extra_doc_1_multithreading.html#autotoc_md19", null ],
      [ "Thread Safety Guidelines", "md_extra_doc_1_multithreading.html#autotoc_md20", null ],
      [ "Thread-Safe Global Instance Implementation", "md_extra_doc_1_multithreading.html#autotoc_md21", [
        [ "Example: OMSimHitManager", "md_extra_doc_1_multithreading.html#autotoc_md22", null ],
        [ "Example: Saving Data Per Thread", "md_extra_doc_1_multithreading.html#autotoc_md23", null ]
      ] ],
      [ "Best Practices for Creating New Thread-Safe Containers", "md_extra_doc_1_multithreading.html#autotoc_md24", null ],
      [ "Troubleshooting Multi-threading Issues", "md_extra_doc_1_multithreading.html#autotoc_md25", [
        [ "1. Use Valgrind Tools", "md_extra_doc_1_multithreading.html#autotoc_md26", null ],
        [ "2. Analyse the Output", "md_extra_doc_1_multithreading.html#autotoc_md27", null ],
        [ "4. Modify and repeat", "md_extra_doc_1_multithreading.html#autotoc_md28", null ]
      ] ]
    ] ],
    [ "Technicalities for Developers", "md_extra_doc_2_technicalities.html", [
      [ "Visualization of Complex Objects", "md_extra_doc_2_technicalities.html#autotoc_md30", null ],
      [ "The Tools namespace", "md_extra_doc_2_technicalities.html#autotoc_md31", null ],
      [ "Nomenclature", "md_extra_doc_2_technicalities.html#autotoc_md33", null ],
      [ "Matching PMT to Measurements", "md_extra_doc_2_technicalities.html#autotoc_md35", [
        [ "Step 1: Fraction of absorbed photons", "md_extra_doc_2_technicalities.html#autotoc_md36", null ],
        [ "Step 2: Expand OMSimPMTResponse and Verify QE", "md_extra_doc_2_technicalities.html#autotoc_md37", null ],
        [ "Step 3: Matching detection efficiency scan", "md_extra_doc_2_technicalities.html#autotoc_md38", null ],
        [ "Step 4: Matching gain / transit time scans", "md_extra_doc_2_technicalities.html#autotoc_md39", null ]
      ] ],
      [ "@code{bash}", "md_extra_doc_2_technicalities.html#autotoc_md40", null ]
    ] ],
    [ "> Effective Area Studies", "md_extra_doc_30_effective_area.html", [
      [ "Introduction to effective areas", "md_extra_doc_30_effective_area.html#autotoc_md42", null ],
      [ "Example using healpy", "md_extra_doc_30_effective_area.html#autotoc_md43", null ]
    ] ],
    [ "> Radioactive Decays Studies", "md_extra_doc_31_radioactive_decays.html", null ],
    [ "> Supernova Studies", "md_extra_doc_32__s_n.html", [
      [ "Input Parameters", "md_extra_doc_32__s_n.html#autotoc_md46", [
        [ "General Parameters", "md_extra_doc_32__s_n.html#autotoc_md47", null ],
        [ "SN Framework Parameters", "md_extra_doc_32__s_n.html#autotoc_md48", null ],
        [ "Fixed Energy Studies", "md_extra_doc_32__s_n.html#autotoc_md49", null ]
      ] ],
      [ "Output Files", "md_extra_doc_32__s_n.html#autotoc_md50", [
        [ "*_info.dat", "md_extra_doc_32__s_n.html#autotoc_md51", null ],
        [ "*_data.dat", "md_extra_doc_32__s_n.html#autotoc_md52", null ]
      ] ],
      [ "Weights", "md_extra_doc_32__s_n.html#autotoc_md53", [
        [ "Interaction Probability Weight:", "md_extra_doc_32__s_n.html#autotoc_md54", null ],
        [ "Flux Weight:", "md_extra_doc_32__s_n.html#autotoc_md55", null ],
        [ "Effective Weight:", "md_extra_doc_32__s_n.html#autotoc_md56", null ]
      ] ]
    ] ],
    [ "Todo List", "todo.html", null ],
    [ "Modules", "modules.html", "modules" ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"5__group__definitions_8h_source.html",
"class_c_a_d_mesh_1_1_file_1_1_lexer.html#ad37669514d188f511c011065ac21a985",
"class_g4_op_boundary_process.html#afea4ec68181011a124e8cdf57bb2214d",
"class_o_m_sim_decays_analysis.html#a715d4897705da383ce1a1d93b68def9b",
"class_o_m_sim_p_m_t_construction.html#a4b59217e8d9ffcae6ffa95ffab8af10f",
"classabc_detector_component.html#a3d796bd0c3a6e22e8bbd11765b98d2e8",
"md_extra_doc_32__s_n.html#autotoc_md55"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';