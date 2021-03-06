/////////////////////////////////////////////////////////////////////////
// $Id: flag_ctrl.cc,v 1.6 2001/10/03 13:10:37 bdenney Exp $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2001  MandrakeSoft S.A.
//
//    MandrakeSoft S.A.
//    43, rue d'Aboukir
//    75002 Paris - France
//    http://www.linux-mandrake.com/
//    http://www.mandrakesoft.com/
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA






#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#define LOG_THIS bx_cpu.





  void
bx_cpu_c::SAHF(BxInstruction_t *i)
{
  set_SF((AH & 0x80) >> 7);
  set_ZF((AH & 0x40) >> 6);
  set_AF((AH & 0x10) >> 4);
  set_CF(AH & 0x01);
  set_PF((AH & 0x04) >> 2);
}

  void
bx_cpu_c::LAHF(BxInstruction_t *i)
{
  AH = (get_SF() ? 0x80 : 0) |
       (get_ZF() ? 0x40 : 0) |
       (get_AF() ? 0x10 : 0) |
       (get_PF() ? 0x04 : 0) |
       (0x02) |
       (get_CF() ? 0x01 : 0);
}

  void
bx_cpu_c::CLC(BxInstruction_t *i)
{
  set_CF(0);
}

  void
bx_cpu_c::STC(BxInstruction_t *i)
{
  set_CF(1);
}

  void
bx_cpu_c::CLI(BxInstruction_t *i)
{
#if BX_CPU_LEVEL >= 2
  if (protected_mode()) {
    if (CPL > IOPL) {
      //BX_INFO(("CLI: CPL > IOPL")); /* ??? */
      exception(BX_GP_EXCEPTION, 0, 0);
      return;
      }
    }
#if BX_CPU_LEVEL >= 3
  else if (v8086_mode()) {
    if (IOPL != 3) {
      //BX_INFO(("CLI: IOPL != 3")); /* ??? */
      exception(BX_GP_EXCEPTION, 0, 0);
      return;
      }
    }
#endif
#endif

  bx_cpu. eflags.if_ = 0;
}

  void
bx_cpu_c::STI(BxInstruction_t *i)
{
#if BX_CPU_LEVEL >= 2
  if (protected_mode()) {
    if (CPL > IOPL) {
      //BX_INFO(("STI: CPL > IOPL")); /* ??? */
      exception(BX_GP_EXCEPTION, 0, 0);
      return;
      }
    }
#if BX_CPU_LEVEL >= 3
  else if (v8086_mode()) {
    if (IOPL != 3) {
      //BX_INFO(("STI: IOPL != 3")); /* ??? */
      exception(BX_GP_EXCEPTION, 0, 0);
      return;
      }
    }
#endif
#endif

  if (!bx_cpu. eflags.if_) {
    bx_cpu. eflags.if_ = 1;
    bx_cpu. inhibit_mask |= BX_INHIBIT_INTERRUPTS;
    bx_cpu. async_event = 1;
    }
}

  void
bx_cpu_c::CLD(BxInstruction_t *i)
{
  bx_cpu. eflags.df = 0;
}

  void
bx_cpu_c::STD(BxInstruction_t *i)
{
  bx_cpu. eflags.df = 1;
}

  void
bx_cpu_c::CMC(BxInstruction_t *i)
{
  set_CF( !get_CF() );
}

  void
bx_cpu_c::PUSHF_Fv(BxInstruction_t *i)
{
  if (v8086_mode() && (IOPL<3)) {
    exception(BX_GP_EXCEPTION, 0, 0);
    return;
    }

#if BX_CPU_LEVEL >= 3
  if (i->os_32) {
    push_32(read_eflags() & 0x00fcffff);
    }
  else
#endif
    {
    push_16(read_flags());
    }
}


  void
bx_cpu_c::POPF_Fv(BxInstruction_t *i)
{

#if BX_CPU_LEVEL >= 3
  if (v8086_mode()) {
    if (IOPL < 3) {
      //BX_INFO(("popf_fv: IOPL < 3"));
      exception(BX_GP_EXCEPTION, 0, 0);
      return;
      }
    if (i->os_32) {
      BX_PANIC(("POPFD(): not supported in virtual mode"));
      exception(BX_GP_EXCEPTION, 0, 0);
      return;
      }
    }

  if (i->os_32) {
    Bit32u eflags;

    pop_32(&eflags);

    eflags &= 0x00277fd7;
    if (!real_mode()) {
      write_eflags(eflags, /* change IOPL? */ CPL==0, /* change IF? */ CPL<=IOPL, 0, 0);
      }
    else { /* real mode */
      write_eflags(eflags, /* change IOPL? */ 1, /* change IF? */ 1, 0, 0);
      }
    }
  else
#endif /* BX_CPU_LEVEL >= 3 */
    { /* 16 bit opsize */
    Bit16u flags;

    pop_16(&flags);

    if (!real_mode()) {
      write_flags(flags, /* change IOPL? */ CPL==0, /* change IF? */ CPL<=IOPL);
      }
    else { /* real mode */
      write_flags(flags, /* change IOPL? */ 1, /* change IF? */ 1);
      }
    }
}


  void
bx_cpu_c::SALC(BxInstruction_t *i)
{
  if ( get_CF() ) {
    AL = 0xff;
    }
  else {
    AL = 0x00;
    }
}
