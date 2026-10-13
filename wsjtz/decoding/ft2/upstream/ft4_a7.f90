! Derived from WS 3.2.1 (261010), lib/ft4/ft4_a7.f90
! Authors: WSJT Development Group; Uwe Risse DG2YCB and contributors.
! GPL-3.0-or-later; see COPYING and NOTICE.md.
! Modified 2026-10-13: isolate FT2 symbols/includes from the WSJT-Z FT4 engine.
module zft2_ft4_a7

  parameter(MAXDEC=200)

! For the following three arrays
!    First index   i=decode number in this sequence
!    Second index  j=0 or 1 for even or odd sequence
!    Third index   k=0 or 1 for previous or current tally for this j
  real dt0_ft4(MAXDEC,0:1,0:1)            !dt0_ft4(i,j,k)
  real f0_ft4(MAXDEC,0:1,0:1)             !f0_ft4(i,j,k)
  character*37 msg0_ft4(MAXDEC,0:1,0:1)   !msg0_ft4(i,j,k)

  integer jseq_ft4                         !even=0, odd=1
  integer ndec_ft4(0:1,0:1)               !ndec_ft4(j,k)
  data ndec_ft4/4*0/,jseq_ft4/0/

contains

subroutine zft2_ft4_a7_save(jseq,dt,f,msg)

  use packjt77
  character*37 msg,msg1
  character*13 w(19)
  character*4 g4
  integer nw(19)
  logical isgrid4

! Statement function:
  isgrid4(g4)=(len_trim(g4).eq.4 .and.                                        &
       ichar(g4(1:1)).ge.ichar('A') .and. ichar(g4(1:1)).le.ichar('R') .and.  &
       ichar(g4(2:2)).ge.ichar('A') .and. ichar(g4(2:2)).le.ichar('R') .and.  &
       ichar(g4(3:3)).ge.ichar('0') .and. ichar(g4(3:3)).le.ichar('9') .and.  &
       ichar(g4(4:4)).ge.ichar('0') .and. ichar(g4(4:4)).le.ichar('9'))

  if(index(msg,'/').ge.1 .or. index(msg,'<').ge.1) go to 999
  call split77(msg,nwords,nw,w)          !Parse msg into words
  if(nwords.lt.1) go to 999
  if(w(1)(1:3).eq.'CQ_') go to 999
  j=jseq

! Add this decode to current table for this sequence
  ndec_ft4(j,1)=ndec_ft4(j,1)+1         !Number of decodes in this sequence
  i=ndec_ft4(j,1)                        !i is index of a new table entry
  if(i.gt.MAXDEC) return                 !Prevent table overflow

  dt0_ft4(i,j,1)=dt                      !Save dt in table
  f0_ft4(i,j,1)=f                        !Save f in table
  msg0_ft4(i,j,1)=trim(w(1))//' '//trim(w(2)) !Save "call_1 call_2"
  if(w(1)(1:3).eq.'CQ ' .and. nw(2).le.2) then
     msg0_ft4(i,j,1)='CQ '//trim(w(2))//' '//trim(w(3)) !Save "CQ DX Call_2"
  endif
  msg1=msg0_ft4(i,j,1)                   !Message without grid
  nn=len(trim(msg1))                      !Message length without grid
! Include grid as part of message
  if(isgrid4(w(nwords))) msg0_ft4(i,j,1)=trim(msg0_ft4(i,j,1))//' '//trim(w(nwords))

! If a transmission at this frequency with message fragment "call_1 call_2"
! was decoded in the previous sequence, flag it as "DO NOT USE" because
! we have already decoded and subtracted that station's next transmission.

  call split77(msg0_ft4(i,j,1),nwords,nw,w)
  do i=1,ndec_ft4(j,0)
     if(f0_ft4(i,j,0).le.-98.0) cycle
     i2=index(msg0_ft4(i,j,0),' '//trim(w(2)))
     if(abs(f-f0_ft4(i,j,0)).le.3.0 .and. i2.ge.3) then
        f0_ft4(i,j,0)=-98.0     !Flag as "do not use" for a potential a7 decode
     endif
  enddo

999 return
end subroutine zft2_ft4_a7_save

subroutine zft2_ft4_a7d(dd0,newdat,call_1,call_2,grid4,xdt,f1,xbase,  &
     nharderrors,dmin,msg37,xsnr)

! Examine the raw data in dd0() for possible "a7" decodes.
! Adapted from ft8_a7d for FT4's 4-FSK/103-symbol frame.

  use timer_module, only: timer
  use packjt77
  include 'ft4_params.f90'
  parameter(NSS=NSPS/NDOWN)              !32 samples per symbol
  parameter(NP2=NMAX/NDOWN)              !4032 downsampled samples
  character*37 msg37,msg,msgsent,msgbest
  character*12 call_1,call_2
  character*4 grid4
  real a(5)
  real s4(0:3,NN)                      !FT4: 4 tones (FT8: s8(0:7,NN))
  real s2(0:255)                       !FT4: 2^8=256 (FT8: s2(0:511))
  real dmm(206)
  real bmeta(174),bmetb(174),bmetc(174),bmetd(174)
  real llra(174),llrb(174),llrc(174),llrd(174)
  real dd0(NMAX)
  real ss(9)
  integer*1 cw(174)
  integer*1 msgbits(77)
  integer*1 nxor(174),hdec(174)
  integer i4tone(NN)                    !FT4: 103 symbols (FT8: itone(79))
  integer ip(1)
  logical one(0:255,0:7)               !FT4: 4-FSK, 8 bits max (FT8: 0:511,0:8)
  integer graymap(0:3)                 !FT4: 4-FSK (FT8: graymap(0:7))
  integer iloc(1)
  complex cd0(0:NP2-1)                 !FT4: NP2=4032 (FT8: 0:3199)
  complex ctwk(2*NSS)                  !FT4: 64 (FT8: ctwk(32))
  complex csymb(NSS)
  complex cs(0:3,NN)                   !FT4: 4 tones, 103 sym (FT8: 0:7,79)
  logical std_1,std_2
  logical first,newdat
  data first/.true./
  data graymap/0,1,3,2/                !FT4: 4-FSK Gray code (FT8: /0,1,3,2,5,6,4,7/)
  save one

  if(first) then
     one=.false.
     do i=0,255                        !FT4: 0:255 (FT8: 0:511)
       do j=0,7                        !FT4: 0:7 (FT8: 0:8)
         if(iand(i,2**j).ne.0) one(i,j)=.true.
       enddo
     enddo
     first=.false.
  endif

  call stdcall(call_1,std_1)
  if(call_1(1:3).eq.'CQ ') std_1=.true.
  call stdcall(call_2,std_2)

  fs2=12000.0/NDOWN                      !666.67 Hz
  dt2=1.0/fs2
  twopi=8.0*atan(1.0)
  delfbest=0.
  ibest=0

  call timer('ft4_dwn ',0)
  call zft2_ft4_downsample(dd0,newdat,f1,cd0)  !FT4 downsample (FT8: ft8_downsample)
  call timer('ft4_dwn ',1)

  i0=nint((xdt+0.5)*fs2)                  !Initial guess for start of signal
  smax=0.0
  do idt=i0-10,i0+10                      !Search over +/- ~15 ms
     call zft2_sync4d(cd0,idt,ctwk,0,sync)     !FT4 sync (FT8: sync8d)
     if(sync.gt.smax) then
        smax=sync
        ibest=idt
     endif
  enddo

! Peak up in frequency
  smax=0.0
  do ifr=-5,5                              !Search over +/- 2.5 Hz
    delf=ifr*0.5
    dphi=twopi*delf*dt2
    phi=0.0
    do i=1,2*NSS                        !FT4: 64 (FT8: 32)
      ctwk(i)=cmplx(cos(phi),sin(phi))
      phi=mod(phi+dphi,twopi)
    enddo
    call zft2_sync4d(cd0,ibest,ctwk,1,sync)   !FT4 sync (FT8: sync8d)
    if( sync .gt. smax ) then
      smax=sync
      delfbest=delf
    endif
  enddo
  a=0.0
  a(1)=-delfbest
  call twkfreq1(cd0,NP2,fs2,a,cd0)
  f1=f1+delfbest                           !Improved estimate of DF

  call timer('ft4_dwn ',0)
  call zft2_ft4_downsample(dd0,.false.,f1,cd0)  !FT4 downsample (FT8: ft8_downsample)
  call timer('ft4_dwn ',1)

! Fine sync search
  smax=0.0
  do idt=-4,4
     call zft2_sync4d(cd0,ibest+idt,ctwk,0,sync) !FT4 sync (FT8: sync8d)
     ss(idt+5)=sync
  enddo
  smax=maxval(ss)
  iloc=maxloc(ss)
  ibest=iloc(1)-5+ibest
  xdt=(ibest-1)*dt2 - 0.5
  sync=smax

! Extract complex symbols and tone powers
! FT4: 103 symbols, 4 tones per symbol (FT8: 79 symbols, 8 tones)
  do k=1,NN
    i1=ibest+(k-1)*NSS
    csymb=cmplx(0.0,0.0)
    if( i1.ge.0 .and. i1+NSS-1 .le. NP2-1 ) csymb=cd0(i1:i1+NSS-1)
    call four2a(csymb,NSS,1,-1,1)
    cs(0:3,k)=csymb(1:4)/1e3          !FT4: bins 1:4 (FT8: 1:8)
    s4(0:3,k)=abs(csymb(1:4))         !FT4: 4 tones (FT8: s8, 8 tones)
  enddo

! Compute bit metrics for 3 data blocks, 3 symbol-sequence lengths
! FT4: 4-FSK, 2 bits/symbol, graymap = /0,1,3,2/
! Data symbols at positions: 5-33 (block 1), 38-66 (block 2), 71-99 (block 3)
  bmeta=0.
  bmetb=0.
  bmetc=0.
  bmetd=0.
  do nseq=1,3
    if(nseq.eq.1) nsym=1
    if(nseq.eq.2) nsym=2
    if(nseq.eq.3) nsym=4
    nt=2**(2*nsym)
    do iblock=1,3
      if(iblock.eq.1) ksoff=4
      if(iblock.eq.2) ksoff=37
      if(iblock.eq.3) ksoff=70
      do k=1,29,nsym
        ks=k+ksoff
        do itone=0,nt-1
          i1=itone/64
          i2=iand(itone,63)/16
          i3=iand(itone,15)/4
          i4=iand(itone,3)
          if(nsym.eq.1) then
            s2(itone)=abs(cs(graymap(i4),ks))
          elseif(nsym.eq.2) then
            s2(itone)=abs(cs(graymap(i3),ks)+cs(graymap(i4),ks+1))
          elseif(nsym.eq.4) then
            s2(itone)=abs(cs(graymap(i1),ks)+cs(graymap(i2),ks+1)+ &
                         cs(graymap(i3),ks+2)+cs(graymap(i4),ks+3))
          endif
        enddo
        i32=1+(k-1)*2+(iblock-1)*58
        if(nsym.eq.1) ibmax=1
        if(nsym.eq.2) ibmax=3
        if(nsym.eq.4) ibmax=7
        do ib=0,ibmax
          bm=maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib)) - &
             maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib))
          if(i32+ib .gt.174) cycle
          if(nseq.eq.1) then
            bmeta(i32+ib)=bm
            den=max(maxval(s2(0:nt-1),one(0:nt-1,ibmax-ib)), &
                    maxval(s2(0:nt-1),.not.one(0:nt-1,ibmax-ib)))
            if(den.gt.0.0) then
              bmetd(i32+ib)=bm/den
            else
              bmetd(i32+ib)=0.0
            endif
          elseif(nseq.eq.2) then
            bmetb(i32+ib)=bm
          elseif(nseq.eq.3) then
            bmetc(i32+ib)=bm
          endif
        enddo
      enddo
    enddo
  enddo
  call normalizebmet(bmeta,174)
  call normalizebmet(bmetb,174)
  call normalizebmet(bmetc,174)
  call normalizebmet(bmetd,174)

  scalefac=2.83
  llra=scalefac*bmeta
  llrb=scalefac*bmetb
  llrc=scalefac*bmetc
  llrd=scalefac*bmetd

  MAXMSG=206
  pbest=0.
  dmin=1.e30
  nharderrors=-1
  i4tone=0

  do imsg=1,MAXMSG
     msg=trim(call_1)//' '//trim(call_2)
     i=imsg
     if(call_1(1:3).eq.'CQ ' .and. i.ne.5) msg='QU1RK '//trim(call_2)
     if(.not.std_1) then
        if(i.eq.1 .or. i.ge.6)  msg='<'//trim(call_1)//'> '//trim(call_2)
        if(i.ge.2 .and. i.le.4) msg=trim(call_1)//' <'//trim(call_2)//'>'
     else if(.not.std_2) then
        if(i.le.4 .or. i.eq.6) msg='<'//trim(call_1)//'> '//trim(call_2)
        if(i.ge.7) msg=trim(call_1)//' <'//trim(call_2)//'>'
     endif
     j0=len(trim(msg))+2
     if(i.eq.2) msg(j0:j0+2)='RRR'
     if(i.eq.3) msg(j0:j0+3)='RR73'
     if(i.eq.4) msg(j0:j0+1)='73'
     if(i.eq.5) then
        if(std_2) then
           msg='CQ '//trim(call_2)
           if(call_1(3:3).eq.'_') msg=trim(call_1)//' '//trim(call_2)
           if(grid4.ne.'RR73') msg=trim(msg)//' '//grid4
        endif
        if(.not.std_2) msg='CQ '//trim(call_2)
     endif
     if(i.eq.6 .and. std_2) msg(j0:j0+3)=grid4
     if(i.ge.7) then
        isnr = -50 + (i-7)/2
        if(iand(i,1).eq.1) then
           write(msg(j0:j0+2),'(i3.2)') isnr
           if(msg(j0:j0).eq.' ') msg(j0:j0)='+'
        else
           write(msg(j0:j0+3),'("R",i3.2)') isnr
           if(msg(j0+1:j0+1).eq.' ') msg(j0+1:j0+1)='+'
        endif
     endif

     call zft2_genft4(msg,0,msgsent,msgbits,i4tone)  !Source-encode this message
     if(msgsent(1:3).eq.'***') cycle             !Skip bad message hypotheses
     call encode174_91(msgbits,cw)                !Get codeword for this message
     pow=0.0
     do i=1,NN
        pow=pow+s4(i4tone(i),i)**2
     enddo

     hdec=0
     where(llra.ge.0.0) hdec=1
     nxor=ieor(hdec,cw)
     da=sum(nxor*abs(llra))

     hdec=0
     where(llrb.ge.0.0) hdec=1
     nxor=ieor(hdec,cw)
     dbb=sum(nxor*abs(llrb))

     hdec=0
     where(llrc.ge.0.0) hdec=1
     nxor=ieor(hdec,cw)
     dc=sum(nxor*abs(llrc))

     hdec=0
     where(llrd.ge.0.0) hdec=1
     nxor=ieor(hdec,cw)
     dd=sum(nxor*abs(llrd))

     dm=min(da,dbb,dc,dd)
     dmm(imsg)=dm
     if(dm.lt.dmin) then
        dmin=dm
        msgbest=msgsent
        pbest=pow
        if(dm.eq.da) then
           nharderrors=count((2*cw-1)*llra.lt.0.0)
        else if(dm.eq.dbb) then
           nharderrors=count((2*cw-1)*llrb.lt.0.0)
        else if(dm.eq.dc) then
           nharderrors=count((2*cw-1)*llrc.lt.0.0)
        else if(dm.eq.dd) then
           nharderrors=count((2*cw-1)*llrd.lt.0.0)
        endif
     endif

  enddo  ! imsg

  iloc=minloc(dmm)
  dmm(iloc(1))=1.e30
  iloc=minloc(dmm)
  dmin2=dmm(iloc(1))
  xsnr=-21.
  arg=pbest/xbase/3.0e6-1.0
  if(arg.gt.0.0) xsnr=max(-22.0,db(arg)-22.0)
  if(dmin.gt.100.0 .or. dmin2/dmin.lt.1.3) nharderrors=-1
  msg37=msgbest
  if(msg37(1:3).eq.'CQ ' .and. std_2 .and. grid4.eq.'    ') nharderrors=-1
  if(msg37(1:6).eq.'QU1RK ') nharderrors=-1

  return
end subroutine zft2_ft4_a7d

end module zft2_ft4_a7
