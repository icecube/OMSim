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
    [ "OMSim Geant4 Framework", "index.html", "index" ],
    [ "Framework functionality", "md_extra__doc_20__common.html", [
      [ "Materials and User Data", "md_extra__doc_20__common.html#autotoc_md2", [
        [ "Material Handling", "md_extra__doc_20__common.html#autotoc_md3", null ],
        [ "Optical Surfaces", "md_extra__doc_20__common.html#autotoc_md4", null ],
        [ "Special Material Types", "md_extra__doc_20__common.html#autotoc_md5", null ],
        [ "Geometry Data", "md_extra__doc_20__common.html#autotoc_md6", null ],
        [ "Adding New Data", "md_extra__doc_20__common.html#autotoc_md7", null ]
      ] ],
      [ "Geometry construction", "md_extra__doc_20__common.html#autotoc_md9", null ],
      [ "Optical Modules and Harness Construction", "md_extra__doc_20__common.html#autotoc_md11", null ],
      [ "Making PMTs and OMs sensitive", "md_extra__doc_20__common.html#autotoc_md13", null ],
      [ "Storing hits and PMT response", "md_extra__doc_20__common.html#autotoc_md14", [
        [ "PMTs Charge, transit time and detection probability", "md_extra__doc_20__common.html#autotoc_md15", null ],
        [ "Hit storage", "md_extra__doc_20__common.html#autotoc_md16", null ]
      ] ],
      [ "Making other volumes sensitive to photons", "md_extra__doc_20__common.html#autotoc_md18", null ],
      [ "Non-terminating sensitive volumes for photon tracking", "md_extra__doc_20__common.html#autotoc_md19", null ]
    ] ],
    [ "Multi-threading mode", "md_extra__doc_21__multithreading.html", [
      [ "Introduction", "md_extra__doc_21__multithreading.html#autotoc_md22", null ],
      [ "Thread Safety Guidelines", "md_extra__doc_21__multithreading.html#autotoc_md23", null ],
      [ "Thread-Safe Global Instance Implementation", "md_extra__doc_21__multithreading.html#autotoc_md24", [
        [ "Example: OMSimHitManager", "md_extra__doc_21__multithreading.html#autotoc_md25", null ],
        [ "Example: Saving Data Per Thread", "md_extra__doc_21__multithreading.html#autotoc_md26", null ]
      ] ],
      [ "Best Practices for Creating New Thread-Safe Containers", "md_extra__doc_21__multithreading.html#autotoc_md27", null ],
      [ "Troubleshooting Multi-threading Issues", "md_extra__doc_21__multithreading.html#autotoc_md28", [
        [ "1. Use Valgrind Tools", "md_extra__doc_21__multithreading.html#autotoc_md29", null ],
        [ "2. Analyse the Output", "md_extra__doc_21__multithreading.html#autotoc_md30", null ],
        [ "4. Modify and repeat", "md_extra__doc_21__multithreading.html#autotoc_md31", null ]
      ] ]
    ] ],
    [ "Technicalities for Developers", "md_extra__doc_22__technicalities.html", [
      [ "Visualization of Complex Objects", "md_extra__doc_22__technicalities.html#autotoc_md33", null ],
      [ "The Tools namespace", "md_extra__doc_22__technicalities.html#autotoc_md34", null ],
      [ "Nomenclature", "md_extra__doc_22__technicalities.html#autotoc_md36", null ],
      [ "Matching PMT to Measurements", "md_extra__doc_22__technicalities.html#autotoc_md38", [
        [ "Step 1: Fraction of absorbed photons", "md_extra__doc_22__technicalities.html#autotoc_md39", null ],
        [ "Step 2: Expand OMSimPMTResponse and Verify QE", "md_extra__doc_22__technicalities.html#autotoc_md40", null ],
        [ "Step 3: Matching detection efficiency scan", "md_extra__doc_22__technicalities.html#autotoc_md41", null ],
        [ "Step 4: Matching gain / transit time scans", "md_extra__doc_22__technicalities.html#autotoc_md42", null ]
      ] ],
      [ "CAD geometries", "md_extra__doc_22__technicalities.html#autotoc_md44", null ]
    ] ],
    [ "> Effective Area Studies", "md_extra__doc_230__effective__area.html", [
      [ "Introduction to effective areas", "md_extra__doc_230__effective__area.html#autotoc_md47", null ],
      [ "Example using healpy", "md_extra__doc_230__effective__area.html#autotoc_md48", null ]
    ] ],
    [ "> Radioactive Decays Studies", "md_extra__doc_231__radioactive__decays.html", null ],
    [ "> Supernova Studies", "md_extra__doc_232___s_n.html", [
      [ "Input Parameters", "md_extra__doc_232___s_n.html#autotoc_md52", [
        [ "General Parameters", "md_extra__doc_232___s_n.html#autotoc_md53", null ],
        [ "SN Framework Parameters", "md_extra__doc_232___s_n.html#autotoc_md54", null ],
        [ "Fixed Energy Studies", "md_extra__doc_232___s_n.html#autotoc_md55", null ]
      ] ],
      [ "Output Files", "md_extra__doc_232___s_n.html#autotoc_md56", [
        [ "*_info.dat", "md_extra__doc_232___s_n.html#autotoc_md57", null ],
        [ "*_data.dat", "md_extra__doc_232___s_n.html#autotoc_md58", null ]
      ] ],
      [ "Weights", "md_extra__doc_232___s_n.html#autotoc_md59", [
        [ "Interaction Probability Weight:", "md_extra__doc_232___s_n.html#autotoc_md60", null ],
        [ "Flux Weight:", "md_extra__doc_232___s_n.html#autotoc_md61", null ],
        [ "Effective Weight:", "md_extra__doc_232___s_n.html#autotoc_md62", null ]
      ] ]
    ] ],
    [ "Todo List", "todo.html", null ],
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
"md_extra__doc_20__common.html#autotoc_md15"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';