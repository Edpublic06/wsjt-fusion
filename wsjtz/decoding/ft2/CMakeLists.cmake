# Isolated WS/Improved FT2 engine. Keep its private symbols and history separate
# from the general WSJT-X FT4 engine so either upstream can be updated alone.
set (WSJTZ_FT2_DIR "${PROJECT_SOURCE_DIR}/wsjtz/decoding/ft2/upstream")
set (WSJTZ_FT2_SOURCES)
foreach (source ft4_decode ft4_a7 ft4_downsample ft4_baseline
    get_ft4_bitmetrics getcandidates4 sync4d subtractft4 genft4 gen_ft4wave)
  list (APPEND WSJTZ_FT2_SOURCES "${WSJTZ_FT2_DIR}/${source}.f90")
endforeach ()
list (APPEND wsjt_FSRCS ${WSJTZ_FT2_SOURCES})
# Each imported source resolves its private ft4_params.f90 in its own directory.
