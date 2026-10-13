! Derived from WS 3.2.1 (261010), lib/ft4/ft4_params.f90
! Authors: WSJT Development Group; Uwe Risse DG2YCB and contributors.
! GPL-3.0-or-later; see COPYING and NOTICE.md.
! Modified 2026-10-13: isolate FT2 symbols/includes from the WSJT-Z FT4 engine.
! FT4 
! LDPC(174,91) code, four 4x4 Costas arrays for sync, ramp-up and ramp-down symbols

parameter (KK=91)                     !Information bits (77 + CRC14)
parameter (ND=87)                     !Data symbols
parameter (NS=16)                     !Sync symbols 
parameter (NN=NS+ND)                  !Sync and data symbols (103)
parameter (NN2=NS+ND+2)               !Total channel symbols (105)
parameter (NSPS=576)                  !Samples per symbol at 12000 S/s
parameter (NZ=NSPS*NN)                !Sync and Data samples (59328)
parameter (NZ2=NSPS*NN2)              !Total samples in shaped waveform (60480)
parameter (NMAX=23*3456)              !Samples in iwave (79488) - sized for FT2 stretched
parameter (NFFT1=2304, NH1=NFFT1/2)   !Length of FFTs for symbol spectra
parameter (NSTEP=NSPS)                !Coarse time-sync step size
parameter (NHSYM=(21*3456-NFFT1)/NSTEP) !Number of symbol spectra (122, based on FT4 signal length)
parameter (NDOWN=18)                  !Downsample factor
