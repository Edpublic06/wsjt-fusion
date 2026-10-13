! WSJT-Z integration regression, 2026-10-13. GPL-3.0-or-later.
! Generate a signal with the existing transmitter, then test the imported
! receiver. The FT4 receiver is exercised separately to catch symbol collisions.
program test_ft2_receiver
  use iso_c_binding, only: c_int64_t
  use wsjtz_ft2_decode, only: ft2_decoder => ft4_decoder
  use ft4_decode, only: ft4_decoder
  implicit none
  integer, parameter :: ft4_count=72576,ft2_count=39744
  type(ft2_decoder) :: receiver
  type(ft4_decoder) :: original_receiver
  real :: wave(ft4_count),offset,got_dt,got_frequency
  complex :: unused(1)
  integer(kind=2) :: audio(ft2_count*2),original_audio(ft4_count)
  integer :: tones(103),depth,i,shift,found,case_id
  integer(kind=1) :: bits(77)
  integer(c_int64_t) :: seed
  character(len=37) :: message,sent
  character(len=12) :: mycall,hiscall

  message='CQ K1JT FN20'
  call genft4(message,0,sent,bits,tones)
  if(sent/=message) error stop 'transmitter message did not encode'
  call gen_ft4wave(tones,103,576,12000.0,750.0,unused,wave,0,ft4_count)
  mycall='W9XYZ'
  hiscall='K1JT'
  do case_id=1,2
    offset=0.25
    if(case_id==2) offset=0.60
    shift=nint(offset*12000.0)
    do depth=1,3
      seed=1234567_c_int64_t
      do i=1,ft2_count
        seed=mod(16807_c_int64_t*seed,2147483647_c_int64_t)
        audio(2*i-1)=int(mod(seed,21_c_int64_t)-10_c_int64_t,kind=2)
      enddo
      do i=1,ft4_count/2
        if(i+shift<=ft2_count) audio(2*(i+shift)-1)= &
             audio(2*(i+shift)-1)+int(2000.0*wave(2*i),kind=2)
      enddo
      audio(2:ft2_count*2:2)=audio(1:ft2_count*2:2)
      found=0
      call receiver%decode(on_ft2,audio,120000+case_id*15,0,750,100,2000,depth, &
           .false.,0,mycall,hiscall,3.75)
      if(found/=1) error stop 'FT2 message missing or duplicated'
      if(abs(got_frequency-1500.0)>3.0) error stop 'FT2 frequency conversion failed'
      if(abs(got_dt-(offset-0.5))>0.06) error stop 'FT2 DT conversion failed'
    enddo
  enddo

  original_audio=0
  shift=6000
  original_audio(shift+1:ft4_count)=int(2000.0*wave(1:ft4_count-shift),kind=2)
  found=0
  call original_receiver%decode(on_ft4,original_audio,0,750,100,2000,3, &
       .false.,0,mycall,hiscall)
  if(found/=1) error stop 'original FT4 receiver regressed'
  print *, 'PASS: FT2 depths 1/2/3, nominal/late signal, DT/frequency, and original FT4'

contains
  subroutine on_ft2(this,sync,snr,dt,freq,decoded,nap,qual)
    class(ft2_decoder), intent(inout) :: this
    real,intent(in) :: sync,dt,freq,qual
    integer,intent(in) :: snr,nap
    character(len=37),intent(in) :: decoded
    if(decoded/=message) return
    found=found+1
    got_dt=0.5*dt-0.25
    got_frequency=2.0*freq
    print *, 'FT2',case_id,depth,trim(decoded),got_dt,got_frequency
  end subroutine
  subroutine on_ft4(this,sync,snr,dt,freq,decoded,nap,qual)
    class(ft4_decoder), intent(inout) :: this
    real,intent(in) :: sync,dt,freq,qual
    integer,intent(in) :: snr,nap
    character(len=37),intent(in) :: decoded
    if(decoded==message) found=found+1
  end subroutine
end program
