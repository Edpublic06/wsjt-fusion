! Fusion adapter to immutable WSJT-X 3.3.0-beta2. GPL-3.0-or-later.
module fusion_legacy_params
  use iso_c_binding
  type, bind(C) :: fusion_params
     integer(c_int) :: nutc
     logical(c_bool) :: ndiskdat
     integer(c_int) :: ntr
     integer(c_int) :: nQSOProgress ! See MainWindow::m_QSOProgress for values
     integer(c_int) :: nfqso
     integer(c_int) :: nftx
     logical(c_bool) :: newdat
     integer(c_int) :: npts8
     integer(c_int) :: nfa
     integer(c_int) :: nfsplit
     integer(c_int) :: nfb
     integer(c_int) :: ntol
     integer(c_int) :: kin
     integer(c_int) :: nzhsym
     integer(c_int) :: nsubmode
     logical(c_bool) :: nagain
     integer(c_int) :: ndepth
     logical(c_bool) :: lft8apon 
     logical(c_bool) :: lapcqonly
     logical(c_bool) :: ljt65apon 
     integer(c_int) :: napwid
     integer(c_int) :: ntxmode
     integer(c_int) :: nmode
     integer(c_int) :: minw
     logical(c_bool) :: nclearave
     integer(c_int) :: minsync
     real(c_float) :: emedelay
     real(c_float) :: dttol
     integer(c_int) :: nlist
     integer(c_int) :: listutc(10)
     integer(c_int) :: n2pass
     integer(c_int) :: nranera
     integer(c_int) :: naggressive
     logical(c_bool) :: nrobust
     integer(c_int) :: nexp_decode
     integer(c_int) :: max_drift
     character(kind=c_char) :: datetime(20)
     character(kind=c_char) :: mycall(12)
     character(kind=c_char) :: mygrid(6)
     character(kind=c_char) :: hiscall(12)
     character(kind=c_char) :: hisgrid(6)
     logical(c_bool) :: b_even_seq
     logical(c_bool) :: b_superfox
     integer(c_int) :: yymmdd

     ! below is ft8md
     character(kind=c_char) :: mybcall(12)
     character(kind=c_char) :: hisbcall(12)
     integer(c_int) :: ncandthin
     integer(c_int) :: ndtcenter
     integer(c_int) :: nft8cycles
     integer(c_int) :: ntrials10
     integer(c_int) :: ntrialsrxf10
     integer(c_int) :: nharmonicsdepth
     integer(c_int) :: ntopfreq65
     integer(c_int) :: nprepass
     integer(c_int) :: nsdecatt
     integer(c_int) :: nlasttx
     integer(c_int) :: ndelay
     integer(c_int) :: nmt
     integer(c_int) :: nft8rxfsens
     integer(c_int) :: nft4depth
     integer(c_int) :: nsecbandchanged
     logical(c_bool) :: nagainfil
     logical(c_bool) :: nstophint
     logical(c_bool) :: nhint
     logical(c_bool) :: fmaskact
     logical(c_bool) :: lmultift8
     logical(c_bool) :: lft8lowth
     logical(c_bool) :: lft8subpass
     logical(c_bool) :: ltxing
     logical(c_bool) :: lhideft8dupes
     logical(c_bool) :: lhound
     logical(c_bool) :: lcommonft8b
     logical(c_bool) :: lmycallstd
     logical(c_bool) :: lhiscallstd
     logical(c_bool) :: lapmyc
     logical(c_bool) :: lmodechanged
     logical(c_bool) :: lbandchanged
     logical(c_bool) :: lenabledxcsearch
     logical(c_bool) :: lwidedxcsearch
     logical(c_bool) :: lmultinst
     logical(c_bool) :: lskiptx1
     integer(c_int) :: ndecoderstart
  end type fusion_params
end module
subroutine fusion_beta2_decode_impl(ss,id2,old,rate,exe,data,temp,share) bind(C)
  use fusion_legacy_params
  use ft8_engine_kernel, only: params_block
  use prog_args
  use timer_impl, only: init_timer
  use iso_c_binding
  implicit none
  real(c_float) :: ss(*)
  integer(c_short) :: id2(*)
  type(fusion_params), intent(in) :: old
  integer(c_int),value :: rate
  character(c_char),intent(in) :: exe(500),data(500),temp(500),share(500)
  type(params_block) :: p
  logical,save :: initialized=.false.
  integer :: i
  if(.not.initialized) then
    do i=1,500
      exe_dir(i:i)=exe(i); data_dir(i:i)=data(i)
      temp_dir(i:i)=temp(i); share_dir(i:i)=share(i)
    enddo
    call init_timer(trim(data_dir)//'/timer-beta2.out')
    initialized=.true.
  endif
  p%nutc=old%nutc
  p%ndiskdat=old%ndiskdat
  p%ntr=old%ntr
  p%nQSOProgress=old%nQSOProgress
  p%nfqso=old%nfqso
  p%nftx=old%nftx
  p%newdat=old%newdat
  p%npts8=old%npts8
  p%nfa=old%nfa
  p%nfsplit=old%nfsplit
  p%nfb=old%nfb
  p%ntol=old%ntol
  p%kin=old%kin
  p%nzhsym=old%nzhsym
  p%nsubmode=old%nsubmode
  p%nagain=old%nagain
  p%ndepth=old%ndepth
  p%lft8apon=old%lft8apon
  p%lapcqonly=old%lapcqonly
  p%ljt65apon=old%ljt65apon
  p%napwid=old%napwid
  p%ntxmode=old%ntxmode
  p%nmode=old%nmode
  p%minw=old%minw
  p%nclearave=old%nclearave
  p%minsync=old%minsync
  p%emedelay=old%emedelay
  p%dttol=old%dttol
  p%nlist=old%nlist
  p%listutc=old%listutc
  p%n2pass=old%n2pass
  p%nranera=old%nranera
  p%naggressive=old%naggressive
  p%nrobust=old%nrobust
  p%nexp_decode=old%nexp_decode
  p%max_drift=old%max_drift
  p%datetime=old%datetime
  p%mycall=old%mycall
  p%mygrid=old%mygrid
  p%hiscall=old%hiscall
  p%hisgrid=old%hisgrid
  p%b_even_seq=old%b_even_seq
  p%b_superfox=old%b_superfox
  p%yymmdd=old%yymmdd
  p%mybcall=old%mybcall
  p%hisbcall=old%hisbcall
  p%ncandthin=old%ncandthin
  p%ndtcenter=old%ndtcenter
  p%nft8cycles=old%nft8cycles
  p%ntrials10=old%ntrials10
  p%ntrialsrxf10=old%ntrialsrxf10
  p%nharmonicsdepth=old%nharmonicsdepth
  p%ntopfreq65=old%ntopfreq65
  p%nprepass=old%nprepass
  p%nsdecatt=old%nsdecatt
  p%nlasttx=old%nlasttx
  p%ndelay=old%ndelay
  p%nmt=old%nmt
  p%nft8rxfsens=old%nft8rxfsens
  p%nft4depth=old%nft4depth
  p%nsecbandchanged=old%nsecbandchanged
  p%nagainfil=old%nagainfil
  p%nstophint=old%nstophint
  p%nhint=old%nhint
  p%fmaskact=old%fmaskact
  p%lmultift8=old%lmultift8
  p%lft8lowth=old%lft8lowth
  p%lft8subpass=old%lft8subpass
  p%ltxing=old%ltxing
  p%lhideft8dupes=old%lhideft8dupes
  p%lhound=old%lhound
  p%lcommonft8b=old%lcommonft8b
  p%lmycallstd=old%lmycallstd
  p%lhiscallstd=old%lhiscallstd
  p%lapmyc=old%lapmyc
  p%lmodechanged=old%lmodechanged
  p%lbandchanged=old%lbandchanged
  p%lenabledxcsearch=old%lenabledxcsearch
  p%lwidedxcsearch=old%lwidedxcsearch
  p%lmultinst=old%lmultinst
  p%lskiptx1=old%lskiptx1
  p%ndecoderstart=old%ndecoderstart
  call multimode_decoder(ss,id2,p,rate)
end subroutine
