# ==============================================================================
# CMakeLists.txt – LumiViz (Konsument von CMakeCraft)
# ==============================================================================
#
# Das Build-System (CMakeCraft) liegt NICHT mehr im Repo: Es wird beim
# Configure in der gepinnten Version (cmakecraft.pin) nach .externals/
# geholt. Was gebaut wird, definiert ausschliesslich Solution.json.
#
#   Version wechseln:            cmakecraft.pin editieren
#   Build-System entwickeln:     -DCMAKECRAFT_LOCAL_DIR=../CMakeCraft
#   Doku:                        CMakeCraft/docs/de/userguides/Neues_Projekt_Guide.md
#
# ==============================================================================

cmake_minimum_required(VERSION 3.25)

include("${CMAKE_CURRENT_LIST_DIR}/CMakeCraftBootstrap.cmake")
