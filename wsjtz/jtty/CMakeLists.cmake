# Sources kept unchanged from WSJT-X v3.3.0-beta1. See source-manifest.json
# and the root NOTICE.md for attribution, revision and licence.
list (APPEND wsjt_FSRCS
  lib/77bit/packjt77_grammar.f90
  lib/jtty/jtty_source_codec.f90
  lib/jtty/jtty_mod.f90
  lib/jtty/jtty_tbcc_code_profile.f90
  lib/jtty/jtty_tbcc_list_decoder.f90
  lib/jtty/tbcc.f90
  lib/jtty/jtty_tbcc_decoder.f90
  lib/jtty/jtty_payload_correlators.f90
  lib/jtty/jtty_fec_mod.f90
  lib/jtty/jttycom.f90
  lib/jtty/jtty_mdecode.f90
  lib/jtty/jtty_receive_context.f90
  lib/jtty/jtty_decode.f90
  lib/jtty/rjtty_sub.f90
  lib/jtty/jtty_peakup.f90
  lib/jtty/jtty_block_pow.f90
  lib/jtty/ana64a.f90
  lib/jtty/genjtty.f90
  lib/jtty/gen_jttywave.f90
  lib/jtty/gen_syncwave.f90
  lib/jtty/subtract_jtty.f90
)
