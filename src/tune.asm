;*************************************************************************************
;*************************************************************************************
;**                                                                                 **
;**  Tinyplayer - LibV2 example                                                     **
;**  written by Tammo 'kb' Hinrichs 2000-2008                                       **
;**  This file is in the public domain                                              **
;**  "Patient Zero" is (C) Melwyn+LB 2005, do not redistribute                      **
;**                                                                                 **
;**  Compile with NASM 0.97 or YASM                                                 **
;**                                                                                 **
;*************************************************************************************
;*************************************************************************************

bits 32
section .data

global _tune
_tune:
  incbin "src/demoooo.v2m"