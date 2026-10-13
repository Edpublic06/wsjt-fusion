! Derived from WS 3.2.1 (261010), lib/ft4_decode.f90
! Authors: WSJT Development Group; Uwe Risse DG2YCB and contributors.
! GPL-3.0-or-later; see COPYING and NOTICE.md.
! Modified 2026-10-13: isolate FT2 symbols/includes from the WSJT-Z FT4 engine.
module wsjtz_ft2_decode

   type :: ft4_decoder
      procedure(ft4_decode_callback), pointer :: callback
   contains
      procedure :: decode
   end type ft4_decoder

   abstract interface
      subroutine ft4_decode_callback (this,sync,snr,dt,freq,decoded,nap,qual)
         import ft4_decoder
         implicit none
         class(ft4_decoder), intent(inout) :: this
         real, intent(in) :: sync
         integer, intent(in) :: snr
         real, intent(in) :: dt
         real, intent(in) :: freq
         character(len=37), intent(in) :: decoded
         integer, intent(in) :: nap
         real, intent(in) :: qual
      end subroutine ft4_decode_callback
   end interface

contains

   subroutine decode(this,callback,iwave,nutc,nQSOProgress,nfqso,    &
      nfa,nfb,ndepth,lapcqonly,ncontest,mycall,hiscall,tperiod)
      use timer_module, only: timer
      use packjt77
      use zft2_ft4_a7
      include 'ft4_params.f90'
      parameter (MAXCAND=200)
      class(ft4_decoder), intent(inout) :: this
      procedure(ft4_decode_callback) :: callback
      parameter (NSS=NSPS/NDOWN,NDMAX=NMAX/NDOWN)
      character message*37,msgsent*37
      character c77*77
      character*37 decodes(100)
      character*17 cdatetime0
      character*12 mycall,hiscall
      character*12 mycall0,hiscall0
      character*6 hhmmss

      complex cd2(0:NDMAX-1)                  !Complex waveform
      complex cb(0:NDMAX-1)
      complex cd(0:NN*NSS-1)                       !Complex waveform
      complex ctwk(2*NSS),ctwk2(2*NSS,-16:16)

      real a(5)
      real bitmetrics(2*NN,5)
      real dd(NMAX)
      real llr(2*ND),llra(2*ND),llrb(2*ND),llrc(2*ND)
      real llrd(2*ND),llre(2*ND),llr_ap(2*ND)
      real candidate(2,MAXCAND)
      real savg(NH1),sbase(NH1)

      integer imetric
      integer, intent(in) :: nutc
      real, intent(in), optional :: tperiod
      integer apbits(2*ND)
      integer apmy_ru(28),aphis_fd(28)
      integer*2 iwave(NMAX)                 !Raw received data
      integer*1 message77(77),rvec(77),apmask(2*ND),cw(2*ND)
      integer*1 message91(91)
      integer*1 hbits(2*NN)
      integer i4tone(103)
      integer nappasses(0:5)    ! # of decoding passes for QSO States 0-5
      integer naptypes(0:5,4)   ! nQSOProgress, decoding pass
      integer mcq(29),mcqru(29),mcqfd(29),mcqtest(29),mcqww(29)
      integer mrrr(19),m73(19),mrr73(19)

      logical nohiscall,unpk77_success
      logical first, dobigfft
      logical dosubtract,doosd
      logical badsync
      logical, intent(in) :: lapcqonly
      logical la7done,newdat_a7
      character*37 msg37_a7,msgsent_a7
      character*12 call_1_a7,call_2_a7
      character*4 grid4_a7
      integer*1 msgbits_a7(77)
      integer i4tone_a7(103)
      integer na7decoded,nharderrors_a7,nsnr_a7
      real xdt_a7,f1_a7,xbase_a7,dmin_a7,xsnr_a7,qual_a7,smax_a7
      real xibest,sm1_sub,sp1_sub,den_sub,delta
      integer, save :: nutc0_ft4=-1

      data first/.true./
      data     mcq/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0/
      data   mcqru/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,1,0,0,1,1,0,0/
      data   mcqfd/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0/
      data mcqtest/0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,1,0,1,1,1,1,1,1,0,0,1,0/
      data   mcqww/0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,1,1,1,1,0/      
      data    mrrr/0,1,1,1,1,1,1,0,1,0,0,1,0,0,1,0,0,0,1/
      data     m73/0,1,1,1,1,1,1,0,1,0,0,1,0,1,0,0,0,0,1/
      data   mrr73/0,1,1,1,1,1,1,0,0,1,1,1,0,1,0,1,0,0,1/
      data rvec/0,1,0,0,1,0,1,0,0,1,0,1,1,1,1,0,1,0,0,0,1,0,0,1,1,0,1,1,0, &
         1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,1,1,1,0,0,1,0,1, &
         0,1,0,1,0,1,1,0,1,1,1,1,1,0,0,0,1,0,1/
      save fs,dt,tt,txt,twopi,h,first,apbits,nappasses,naptypes, &
         mycall0,hiscall0,ctwk2

      this%callback => callback
      hhmmss=cdatetime0(8:13)
      dxcall13=hiscall        ! initialize for use in packjt77
      mycall13=mycall

      smax1=0.
      nd1=0
      nd2=0
      nd3=0
      na7decoded=0

      if(first) then
         fs=12000.0/NDOWN                !Sample rate after downsampling
         dt=1/fs                         !Sample interval after downsample (s)
         tt=NSPS*dt                      !Duration of "itone" symbols (s)
         txt=NZ*dt                       !Transmission length (s) without ramp up/down
         twopi=8.0*atan(1.0)
         h=1.0

         do idf=-16,16
            a=0.
            a(1)=real(idf)
            ctwk=1.
            call twkfreq1(ctwk,2*NSS,fs/2.0,a,ctwk2(:,idf))
         enddo

         mcq=2*mod(mcq+rvec(1:29),2)-1
         mcqru=2*mod(mcqru+rvec(1:29),2)-1
         mcqfd=2*mod(mcqfd+rvec(1:29),2)-1
         mcqtest=2*mod(mcqtest+rvec(1:29),2)-1
         mcqww=2*mod(mcqww+rvec(1:29),2)-1
         mrrr=2*mod(mrrr+rvec(59:77),2)-1
         m73=2*mod(m73+rvec(59:77),2)-1
         mrr73=2*mod(mrr73+rvec(59:77),2)-1
         nappasses(0)=2
         nappasses(1)=2
         nappasses(2)=2
         nappasses(3)=2
         nappasses(4)=2
         nappasses(5)=3

! iaptype
!------------------------
!   1        CQ     ???    ???           (29 ap bits)
!   2        MyCall ???    ???           (29 ap bits)
!   3        MyCall DxCall ???           (58 ap bits)
!   4        MyCall DxCall RRR           (77 ap bits)
!   5        MyCall DxCall 73            (77 ap bits)
!   6        MyCall DxCall RR73          (77 ap bits)
!********
         naptypes(0,1:4)=(/1,2,0,0/) ! Tx6 selected (CQ)
         naptypes(1,1:4)=(/2,3,0,0/) ! Tx1
         naptypes(2,1:4)=(/2,3,0,0/) ! Tx2
         naptypes(3,1:4)=(/3,6,0,0/) ! Tx3
         naptypes(4,1:4)=(/3,6,0,0/) ! Tx4
         naptypes(5,1:4)=(/3,1,2,0/) ! Tx5

         mycall0=''
         hiscall0=''
         first=.false.
      endif

      l1=index(mycall,char(0))
      if(l1.ne.0) mycall(l1:)=" "
      l1=index(hiscall,char(0))
      if(l1.ne.0) hiscall(l1:)=" "
      if(mycall.ne.mycall0 .or. hiscall.ne.hiscall0) then
         apbits=0
         apbits(1)=99
         apbits(30)=99
         apmy_ru=0
         aphis_fd=0

         if(len(trim(mycall)) .lt. 3) go to 10

         nohiscall=.false.
         hiscall0=hiscall
         if(len(trim(hiscall0)).lt.3) then
            hiscall0=mycall  ! use mycall for dummy hiscall - mycall won't be hashed.
            nohiscall=.true.
         endif
         message=trim(mycall)//' '//trim(hiscall0)//' RR73'
         i3=-1
         n3=-1
         call pack77(message,i3,n3,c77)
         call unpack77(c77,1,msgsent,unpk77_success)
         if(i3.ne.1 .or. (message.ne.msgsent) .or. .not.unpk77_success) go to 10
         read(c77,'(77i1)') message77
         apmy_ru=2*mod(message77(1:28)+rvec(2:29),2)-1
         aphis_fd=2*mod(message77(30:57)+rvec(29:56),2)-1
         message77=mod(message77+rvec,2)
         call encode174_91(message77,cw)
         apbits=2*cw-1
         if(nohiscall) apbits(30)=99

10       continue
         mycall0=mycall
         hiscall0=hiscall
      endif
      ndecodes=0
      decodes=' '
      fa=nfa
      fb=nfb
      dd=iwave
! Zero samples beyond FT4 signal length when called for FT4
! (NMAX is sized for FT2 stretched; FT4 only fills 21*3456 samples)
      if(.not.present(tperiod) .or. tperiod.gt.4.0) then
         dd(21*3456+1:)=0.
      endif

! A7 sequence tracking: determine even/odd sequence.
! nutc is HHMMSS. tperiod is 7.5 for FT4, 3.75 for FT2.
      isec=mod(nutc,100)
      if(present(tperiod)) then
         jseq_ft4=mod(nint(real(isec)/tperiod),2)
      else
         jseq_ft4=mod(nint(real(isec)/7.5),2)
      endif
      if(nutc0_ft4.eq.-1) then
         msg0_ft4=' '
         dt0_ft4=0.
         f0_ft4=0.
      endif
      if(nutc.ne.nutc0_ft4) then
         iz=ndec_ft4(jseq_ft4,1)
         if(iz.gt.0) then
            dt0_ft4(1:iz,jseq_ft4,0)  = dt0_ft4(1:iz,jseq_ft4,1)
            f0_ft4(1:iz,jseq_ft4,0)   = f0_ft4(1:iz,jseq_ft4,1)
            msg0_ft4(1:iz,jseq_ft4,0) = msg0_ft4(1:iz,jseq_ft4,1)
         endif
         ndec_ft4(jseq_ft4,0)=iz
         ndec_ft4(jseq_ft4,1)=0
         nutc0_ft4=nutc
         dt0_ft4(:,jseq_ft4,1)=0.
         f0_ft4(:,jseq_ft4,1)=0.
      endif

! ndepth=3: 3 passes, bp+osd
! ndepth=2: 3 passes, bp only
! ndepth=1: 1 pass, no subtraction

      max_iterations=80
      syncmin=1.18
	  if(tperiod.eq.3.75) syncmin=1
      dosubtract=.true.
      doosd=.true.
      nsp=3
      if(ndepth.eq.2) then
         doosd=.false.
      endif
      if(ndepth.eq.1) then
         nsp=1
         dosubtract=.false.
         doosd=.false.
      endif

      la7done=.false.
      do isp = 1,nsp
         if(isp.eq.2) then
            if(ndecodes.eq.0) exit
            nd1=ndecodes
         elseif(isp.eq.3) then
            nd2=ndecodes-nd1
            if(nd2.eq.0) exit
         endif

         if(isp.le.2) then
            imetric=1
         else
            imetric=2
         endif

         call decode_pass(imetric,nnew)
      enddo                               !Subtraction loop
      nd3=ndecodes-nd1-nd2                  !New decodes from pass 3

! ============ A7 decoding stage ============
! Attempt a7 hypothesis-based decoding using callsign pairs from
! saved decodes of the previous FT4 sequence.  Successful a7 decodes
! are subtracted.
      if(.not.la7done .and. ndepth.ge.2 .and.                          &
           ndec_ft4(jseq_ft4,0).ge.1 .and. ncontest.lt.6) then
         la7done=.true.
         na7decoded=0
         newdat_a7=.true.

         do ia7=1,ndec_ft4(jseq_ft4,0)
            if(f0_ft4(ia7,jseq_ft4,0).eq.-99.0) exit
            if(f0_ft4(ia7,jseq_ft4,0).eq.-98.0) cycle
            if(index(msg0_ft4(ia7,jseq_ft4,0),'<').ge.1) cycle

! Parse saved message into call_1, call_2, grid4
            msg37_a7=msg0_ft4(ia7,jseq_ft4,0)
            i1a7=index(msg37_a7,' ')
            if(i1a7.lt.2) cycle
            i2a7=index(msg37_a7(i1a7+1:),' ') + i1a7
            call_1_a7=msg37_a7(1:i1a7-1)
            call_2_a7=msg37_a7(i1a7+1:i2a7-1)
            grid4_a7=msg37_a7(i2a7+1:i2a7+4)
            if(grid4_a7.eq.'RR73' .or. index(grid4_a7,'+').gt.0 .or.   &
                 index(grid4_a7,'-').gt.0) grid4_a7='    '

            xdt_a7=dt0_ft4(ia7,jseq_ft4,0)
            f1_a7=f0_ft4(ia7,jseq_ft4,0)

! Compute xbase for SNR estimation from sbase (linear power for FT4)
            ixb=max(1,nint(f1_a7/(12000.0/NFFT1)))
            if(ixb.ge.1 .and. ixb.le.NH1) then
               xbase_a7=sbase(ixb)
            else
               xbase_a7=1.0
            endif

            msg37_a7='                                     '
            call timer('zft2_ft4_a7d ',0)
            call zft2_ft4_a7d(dd,newdat_a7,call_1_a7,call_2_a7,grid4_a7,    &
                 xdt_a7,f1_a7,xbase_a7,nharderrors_a7,dmin_a7,         &
                 msg37_a7,xsnr_a7)
            call timer('zft2_ft4_a7d ',1)
            if(newdat_a7) newdat_a7=.false.

            if(nharderrors_a7.ge.0) then
! Check for duplicate
               idupe=0
               do id=1,ndecodes
                  if(decodes(id).eq.msg37_a7) idupe=1
               enddo
               if(idupe.eq.0) then
                  ndecodes=ndecodes+1
                  decodes(ndecodes)=msg37_a7
                  na7decoded=na7decoded+1

! Subtract the a7-decoded signal
                  if(dosubtract) then
                     call zft2_genft4(msg37_a7,0,msgsent_a7,msgbits_a7,      &
                          i4tone_a7)
                     call timer('subtract',0)
                     call zft2_subtractft4(dd,i4tone_a7,f1_a7,xdt_a7+0.5)
                     call timer('subtract',1)
                  endif

! Report the decode
                  nsnr_a7=nint(max(-21.0,xsnr_a7))
                  iaptype=7
                  qual_a7=1.0
                  smax_a7=0.0
                  call this%callback(smax_a7,nsnr_a7,xdt_a7,f1_a7,     &
                       msg37_a7,iaptype,qual_a7)
                  call zft2_ft4_a7_save(jseq_ft4,xdt_a7,f1_a7,msg37_a7)
               endif
            endif
         enddo                          !a7 decode loop

      endif

! ============ Post-A7 pass (pass 4) ============
! After pass 3 and A7 subtraction, search for signals that were
! hidden underneath the subtracted signals.
      if(dosubtract .and. (nd3.gt.0 .or. na7decoded.gt.0)) then
         imetric=2
         call decode_pass(imetric,nnew)
      endif

      return

   contains

      subroutine decode_pass(imetric_pass,nnew)
! Search for candidates, attempt sync/decode, subtract on success.
! Uses host association for all shared state (dd, decodes, etc.).
         integer, intent(in) :: imetric_pass
         integer, intent(out) :: nnew
         integer ndec0

         ndec0=ndecodes
         candidate=0.0
         ncand=0
         call timer('getcand4',0)
         call zft2_getcandidates4(dd,fa,fb,syncmin,nfqso,MAXCAND,savg,candidate,   &
            ncand,sbase)
         call timer('getcand4',1)
         dobigfft=.true.
         do icand=1,ncand
            f0=candidate(1,icand)
            snr=candidate(2,icand)-1.0
            call timer('ft4_down',0)
            call zft2_ft4_downsample(dd,dobigfft,f0,cd2)  !Downsample to 32 Sam/Sym
            call timer('ft4_down',1)
            if(dobigfft) dobigfft=.false.
            sum2=sum(cd2*conjg(cd2))/(real(NMAX)/real(NDOWN))
            if(sum2.gt.0.0) cd2=cd2/sqrt(sum2)
! Sample rate is now 12000/18 = 666.67 samples/second
            do iseg=1,3                ! DT search is done over 3 segments
               do isync=1,2          
                  if(isync.eq.1) then
                     idfmin=-12
                     idfmax=12
                     idfstp=3
                     ibmin=-344
                     ibmax=1012
                     if(iseg.eq.1) then
                        ibmin=108
                        ibmax=560
                     elseif(iseg.eq.2) then
                        smax1=smax
                        ibmin=560
                        ibmax=1266     ! was 1066; 1266 means DT until +1,40s instead +1.02 (JTDX has 1266)
                     elseif(iseg.eq.3) then
                        ibmin=-344
                        ibmax=108
                     endif
                     ibstp=4
                  else
                     idfmin=idfbest-4
                     idfmax=idfbest+4
                     idfstp=1
                     ibmin=ibest-5
                     ibmax=ibest+5
                     ibstp=1
                  endif
                  ibest=-1
                  idfbest=0
                  smax=-99.
                  call timer('zft2_sync4d  ',0)
                  do idf=idfmin,idfmax,idfstp
                     do istart=ibmin,ibmax,ibstp
                        call zft2_sync4d(cd2,istart,ctwk2(:,idf),1,sync)  !Find sync power
                        if(sync.gt.smax) then
                           smax=sync
                           ibest=istart
                           idfbest=idf
                        endif
                     enddo
                  enddo
                  call timer('zft2_sync4d  ',1)
               enddo
               if(iseg.eq.1) smax1=smax
               if(smax.lt.1.2) cycle
               if(iseg.gt.1 .and. smax.lt.smax1) cycle 
               f1=f0+real(idfbest)
               if( f1.le.10.0 .or. f1.ge.4990.0 ) cycle
               call timer('ft4down ',0)
               call zft2_ft4_downsample(dd,dobigfft,f1,cb) !Final downsample, corrected f0
               call timer('ft4down ',1)
               sum2=sum(abs(cb)**2)/(real(NSS)*NN)
               if(sum2.gt.0.0) cb=cb/sqrt(sum2)
               cd=0.
               if(ibest.ge.0) then
                  it=min(NDMAX-1,ibest+NN*NSS-1)
                  np=it-ibest+1
                  cd(0:np-1)=cb(ibest:it)
               else
                  cd(-ibest:ibest+NN*NSS-1)=cb(0:NN*NSS+2*ibest-1)
               endif
               call timer('bitmet  ',0)
               call zft2_get_ft4_bitmetrics(cd,imetric_pass,bitmetrics,badsync)
               call timer('bitmet  ',1)
               if(badsync) cycle
               hbits=0
               where(bitmetrics(:,1).ge.0) hbits=1
               ns1=count(hbits(  1:  8).eq.(/0,0,0,1,1,0,1,1/))
               ns2=count(hbits( 67: 74).eq.(/0,1,0,0,1,1,1,0/))
               ns3=count(hbits(133:140).eq.(/1,1,1,0,0,1,0,0/))
               ns4=count(hbits(199:206).eq.(/1,0,1,1,0,0,0,1/))
               nsync_qual=ns1+ns2+ns3+ns4
               if(nsync_qual.lt. 20) cycle

! Sub-sample DT refinement via 3-point parabolic interpolation
               if(ibest.gt.0 .and. ibest.lt.NDMAX-1) then
                  call zft2_sync4d(cd2,ibest-1,ctwk2(:,idfbest),1,sm1_sub)
                  call zft2_sync4d(cd2,ibest+1,ctwk2(:,idfbest),1,sp1_sub)
                  den_sub=sm1_sub - 2.0*smax + sp1_sub
                  if(den_sub.lt.-1.0e-6) then
                     delta=0.5*(sm1_sub - sp1_sub)/den_sub
                     delta=max(-0.5,min(0.5,delta))
                     xibest=real(ibest) + delta
                  else
                     xibest=real(ibest)
                  endif
               else
                  xibest=real(ibest)
               endif

               scalefac=2.83
               llra(  1: 58)=bitmetrics(  9: 66, 1)
               llra( 59:116)=bitmetrics( 75:132, 1)
               llra(117:174)=bitmetrics(141:198, 1)
               llra=scalefac*llra
               llrb(  1: 58)=bitmetrics(  9: 66, 2)
               llrb( 59:116)=bitmetrics( 75:132, 2)
               llrb(117:174)=bitmetrics(141:198, 2)
               llrb=scalefac*llrb
               llrc(  1: 58)=bitmetrics(  9: 66, 3)
               llrc( 59:116)=bitmetrics( 75:132, 3)
               llrc(117:174)=bitmetrics(141:198, 3)
               llrc=scalefac*llrc
               llrd(  1: 58)=bitmetrics(  9: 66, 4)
               llrd( 59:116)=bitmetrics( 75:132, 4)
               llrd(117:174)=bitmetrics(141:198, 4)
               llrd=scalefac*llrd
               llre(  1: 58)=bitmetrics(  9: 66, 5)
               llre( 59:116)=bitmetrics( 75:132, 5)
               llre(117:174)=bitmetrics(141:198, 5)
               llre=scalefac*llre

               apmag=maxval(abs(llra))*1.1
               npasses=5+nappasses(nQSOProgress)
               if(lapcqonly) npasses=6
               if(ndepth.eq.1) npasses=5
               if(ncontest.ge.6) npasses=5  ! Don't support Fox and Hound
               do ipass=1,npasses
                  if(ipass.eq.1) llr=llra
                  if(ipass.eq.2) llr=llrb
                  if(ipass.eq.3) llr=llrc
                  if(ipass.eq.4) llr=llrd
                  if(ipass.eq.5) llr=llre
                  if(ipass.le.5) then
                     apmask=0
                     iaptype=0
                  endif

                  if(ipass .gt. 5) then
                     llr_ap=llrc
                     iaptype=naptypes(nQSOProgress,ipass-5)
                     if(lapcqonly) iaptype=1

! ncontest=0 : NONE
!          1 : NA_VHF
!          2 : EU_VHF
!          3 : FIELD DAY
!          4 : RTTY
!          5 : WW_DIGI 
!          6 : FOX
!          7 : HOUND
!
! Conditions that cause us to bail out of AP decoding
                     napwid=50
                     if(ncontest.le.5 .and. iaptype.ge.3 .and. (abs(f1-nfqso).gt.napwid) ) cycle
                     if(iaptype.ge.2 .and. apbits(1).gt.1) cycle  ! No, or nonstandard, mycall
                     if(iaptype.ge.3 .and. apbits(30).gt.1) cycle ! No, or nonstandard, dxcall

                     if(iaptype.eq.1) then  ! CQ or CQ TEST or CQ FD or CQ RU or CQ WW
                        apmask=0
                        apmask(1:29)=1
                        if( ncontest.eq.0 ) llr_ap(1:29)=apmag*mcq(1:29)
                        if( ncontest.eq.1 ) llr_ap(1:29)=apmag*mcqtest(1:29)
                        if( ncontest.eq.2 ) llr_ap(1:29)=apmag*mcqtest(1:29)
                        if( ncontest.eq.3 ) llr_ap(1:29)=apmag*mcqfd(1:29)
                        if( ncontest.eq.4 ) llr_ap(1:29)=apmag*mcqru(1:29)
                        if( ncontest.eq.5 ) llr_ap(1:29)=apmag*mcqww(1:29)
                     endif

                     if(iaptype.eq.2) then ! MyCall,???,???
                        apmask=0
                        if(ncontest.eq.0.or.ncontest.eq.1.or.ncontest.eq.5) then
                           apmask(1:29)=1
                           llr_ap(1:29)=apmag*apbits(1:29)
                        else if(ncontest.eq.2) then
                           apmask(1:28)=1
                           llr_ap(1:28)=apmag*apbits(1:28)
                        else if(ncontest.eq.3) then
                           apmask(1:28)=1
                           llr_ap(1:28)=apmag*apbits(1:28)
                        else if(ncontest.eq.4) then
                           apmask(2:29)=1
                           llr_ap(2:29)=apmag*apmy_ru(1:28)
                        endif
                     endif

                     if(iaptype.eq.3) then ! MyCall,DxCall,???
                        apmask=0
                        if(ncontest.eq.0.or.ncontest.eq.1.or.ncontest.eq.2.or.ncontest.eq.5) then
                           apmask(1:58)=1
                           llr_ap(1:58)=apmag*apbits(1:58)
                        else if(ncontest.eq.3) then ! Field Day
                           apmask(1:56)=1
                           llr_ap(1:28)=apmag*apbits(1:28)
                           llr_ap(29:56)=apmag*aphis_fd(1:28)
                        else if(ncontest.eq.4) then 
                           apmask(2:57)=1
                           llr_ap(2:29)=apmag*apmy_ru(1:28)
                           llr_ap(30:57)=apmag*apbits(30:57)
                        endif
                     endif

                     if(iaptype.eq.4 .or. iaptype.eq.5 .or. iaptype.eq.6) then
                        apmask=0
                        if(ncontest.le.5) then
                           apmask(1:77)=1   ! mycall, hiscall, RRR|73|RR73
                           if(iaptype.eq.6) llr_ap(1:77)=apmag*apbits(1:77)
                        endif
                     endif

                     llr=llr_ap
                  endif
                  message77=0
                  dmin=0.0

                  ndeep=2
                  maxosd=2  
                  if(abs(nfqso-f1).le.napwid) then
                     ndeep=2
                     maxosd=3
                  endif
                  if(.not.doosd) maxosd = -1
                  call timer('dec174_91 ',0)
                  Keff=91
                  call decode174_91(llr,Keff,maxosd,ndeep,apmask,message91,cw, &
                                    ntype,nharderror,dmin)
                  message77=message91(1:77)
                  call timer('dec174_91 ',1)

                  if(sum(message77).eq.0) cycle
                  if( nharderror.ge.0 ) then
                     message77=mod(message77+rvec,2) ! remove rvec scrambling
                     write(c77,'(77i1)') message77(1:77)
                     call unpack77(c77,1,message,unpk77_success)
                     if(.not.unpk77_success) exit
                     if(dosubtract) then
                        call zft2_get_ft4_tones_from_77bits(message77,i4tone)
                        dt=xibest/fs
                        call timer('subtract',0)
                        call zft2_subtractft4(dd,i4tone,f1,dt)
                        call timer('subtract',1)
                     endif
                     idupe=0
                     do i=1,ndecodes
                        if(decodes(i).eq.message) idupe=1
                     enddo
                     if(idupe.eq.1) exit
                     ndecodes=ndecodes+1
                     decodes(ndecodes)=message
                     if(snr.gt.0.0) then
                        xsnr=10*log10(snr)-14.8
                     else
                        xsnr=-21.0
                     endif
                     nsnr=nint(max(-21.0,xsnr))
                     xdt=xibest/fs - 0.5
                     qual=1.0-(nharderror+dmin)/60.0 
                     call this%callback(smax,nsnr,xdt,f1,message,iaptype,qual)
                     call zft2_ft4_a7_save(jseq_ft4,xdt,f1,message)
                     exit
                  endif
               enddo                      !Sequence estimation
               if(nharderror.ge.0) exit
            enddo                         !3 DT segments
         enddo                            !Candidate list
         nnew=ndecodes-ndec0
      end subroutine decode_pass

   end subroutine decode

end module wsjtz_ft2_decode
