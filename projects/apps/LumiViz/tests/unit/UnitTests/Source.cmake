# ==============================================================================
# Source.cmake for tests/unit/
# CMake Architecture V2 - App-Container Template
# ==============================================================================
#
# Unit Tests:
#   - Fast, isolated tests
#   - No external dependencies
#   - Run on every build
#
# ==============================================================================


# Set local lists for this directory
set(_local_sources
    # (sources - *.c; *.cpp)
    "${CMAKE_CURRENT_LIST_DIR}/test_main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ServiceContainer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_EventBus.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_CommandBus.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PipelineSchema.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_EqualizerMigration.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PulsingMigration.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_WaveformMigration.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_OscilloscopeMigration.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_SuperscopeMigration.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ColorGradientModule.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LuaScriptEngine.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_EelTranspiler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_AlbumCover.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_AvsParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkdropSamplerName.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkParser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkScriptContract.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkdropPreset.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkShaderClassifier.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkdropSerializer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_HlslTranspiler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkdropGlSmoke.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_BloomGlSmoke.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Scope3DGlSmoke.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Screenshot.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Shortcuts.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_TexerIIGlSmoke.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ListEnabledGlSmoke.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_DmoveFixpunkt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MovementScheiben.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_AvsChainTranslator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ChainSerializer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_NodePresetStore.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShaderVorlagen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_FieldDocs.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_HandbuchBeispiele.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_FieldBindings.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_TextTags.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LyricsModel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_SongtextQuelle.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_FeldSteuerungGl.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_FieldInventory.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ParamScript.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ScriptVariablesGl.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_EffectPalette.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_HostSwitcher.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerBelege.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerMischer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerWiedergabe.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerNahtR3.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerBildquelle.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerMessung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerSitzung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_AchseBilder.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerBearbeiten.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerPegelform.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerFormPresets.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerMute.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerVorlagen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerSichttestStart.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerTonfuehrung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerNahtTon.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerBelegeTon.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ComposerNahtMixer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopBelegeTon.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopInstrument.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopSample.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopGrundton.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopBelegeAufnahme.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeBelege.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeKontur.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeKnoten.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeTon.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeZeichnen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeFenster.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeSvg.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeBild.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShapeSpruenge.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_XyScopeKnoten.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopAufnahme.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopAufnahmeFenster.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopDrumQuellen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopAuswahl.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopBausteine.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopWerte.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopClipEffekte.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopEffekte.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopInstrumentEditor.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopInstrumentPalette.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopKits.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopKitFenster.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopTakteSprung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopEffekteSchalter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopKern.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopWiedergabe.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopSpieler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopArrangement.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_LoopDrumEditor.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PaletteUi.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkdropGestaltung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_NseelZufall.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkdropFft.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MilkdropAudio.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_EigeneUniforms.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ScriptContext.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ScriptFormatter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_MeshWarpWrapper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_GpuParticlesWrapper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PixelFilterWrapper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ScriptModules.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_BeatEstimator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_GpuPreference.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Mitgeliefert.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Suchordner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ZipArchiv.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Datensicherung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_DatensicherungDialoge.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PublishSammler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_PublishPacken.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_EffectChain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShadertoyWrapper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_ShaderVertrag.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_IsfImport.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_IsfFilterWrapper.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_IsfGlSmoke.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_Playlist.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test_VisualizerPresetManager.cpp"
)
set(_local_headers
    # (headers - *.h; *.hpp)
)
set(_local_templates
    # (no templates - *.t; *.tpp)
)
set(_local_inlines
    # (no inlines - *.inl)
)
set(_local_impl
    # (no impl - *.impl)
)
set(_local_modules
    # (no modules - *.cxx)
)
set(_local_includes
    # (no impl - *.?)
)

dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found modules  : ${_local_modules}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found includes : ${_local_includes}" ID DEB_FOUND_MSG)

# MSVC: test_ChainSerializer.cpp sprengt seit dem G2-Knoten (S69, 88 Typen im
# Varianten-Roundtrip) das COFF-Abschnittslimit (fatal error C1128 im
# Debug-Build) — /bigobj nur fuer diese Datei (Muster panels/Source.cmake).
set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/test_ChainSerializer.cpp"
    PROPERTIES COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")

# Aggregate to parent scope
list(APPEND ${TARGET_NAME}_SOURCES ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL ${_local_impl})
list(APPEND ${TARGET_NAME}_MODULES ${_local_modules})
list(APPEND ${TARGET_NAME}_INCLUDES ${_local_includes})

dbg(DBG_NORMAL "Aggregated sources  : ${${TARGET_NAME}_SOURCES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated headers  : ${${TARGET_NAME}_HEADERS}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated templates: ${${TARGET_NAME}_TEMPLATES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated inlines  : ${${TARGET_NAME}_INLINES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated impl     : ${${TARGET_NAME}_IMPL}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated modules  : ${${TARGET_NAME}_MODULES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated includes : ${${TARGET_NAME}_INCLUDES}" ID DEB_AGG)

# Optional cleanup (cosmetic)
unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
unset(_local_modules)
unset(_local_includes)

dbg(DBG_ULTRA_RARE "include subfolders:" ID INCLUDE_MSG)
# Include subfolders recursively (activate as needed)
# include("${CMAKE_CURRENT_LIST_DIR}/subfolder/Source.cmake")

