/*
 *  XERIS/APEX System I
 *  Autonomous Poly Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  md_(arch)_(dev)_def.h (arch/avr5/mech/dev/mech_avr5.h)
 *
 *  ATmega328p
 */

#ifndef __XERIS_H_M_avr5_atmega328p_DEF
#define __XERIS_H_M_avr5_atmega328p_DEF

// Architecture configs
#define c_RAMSTART      0x0100
#define c_RAMEND        0x08ff
#define c_FLASHSTART    0x0000
#define c_FLASHEND      0x7fff
#define c_INTMAX        25

// System register pointers and bit locations

/* UART0 */
// Registers
#define r_udr   0xc6      //USART I/O data register
#define r_ucsra 0xc0      //USART Control and Status
#define r_ucsrb 0xc1
#define r_ucsrc 0xc2
#define r_ubrrl 0xc4      // USART Baud Rate Low
#define r_ubrrh 0xc5      // High

// bits - datasheet pg 191
#define b_ucsra_rxc     ( 1 << 7 )      //RX complete
#define b_ucsra_txc     ( 1 << 6 )      //TX complete
#define b_ucsra_udre    ( 1 << 5 )      //tx buffer empty
#define b_ucsra_u2x     ( 1 << 1 )      //double rate
#define b_ucsrb_rxen    ( 1 << 4 )      //RX enable
#define b_ucsrb_txen    ( 1 << 3 )      //TX enable
#define b_ucsrb_ucsz2   ( 1 << 2 )      // byte size bit 2
#define b_ucsrc_umsel1  ( 1 << 7 )      // mode select
#define b_ucsrc_umsel0  ( 1 << 6 )
#define b_ucsrc_upm1    ( 1 << 5 )      // parity
#define b_ucsrc_upm0    ( 1 << 4 )
#define b_ucsrc_usbs    ( 1 << 3 )      // stop bit
#define b_ucsrc_ucsz1   ( 1 << 2 )      // byte size
#define b_ucsrc_ucsz0   ( 1 << 1 )
#define b_ucsrc_ucpol   ( 1 )           // clock polrity

/* SPI */
// Registers
#define r_spcr  0x2c        // Control Register
#define r_spsr  0x2d        // Status Register
#define r_spdr  0x2e        // Data Register

// bits - datasheet pg 167
#define b_spcr_spe      ( 1 << 6 )      // SPI enable
#define b_spcr_dord     ( 1 << 5 )      // Data order
#define b_spcr_mstr     ( 1 << 4 )      // Master/slave select
#define b_spcr_cpol     ( 1 << 3 )      // clock polarity
#define b_spcr_cpha     ( 1 << 2 )      // clock phase
#define b_spcr_spr1     ( 1 << 1 )      // clock rate
#define b_spcr_spr0     ( 1 )

#define b_spsr_spif     ( 1 << 7 )      // SPI Interrupt flag
#define b_spsr_wcol     ( 1 << 6 )      // Write collision
#define b_spsr_spi2x    ( 1 )           // Double-speed mode

/* PORTS */
// Registers
#define r_portb     0x05
#define r_ddrb      0x04
#define r_pinb      0x03

#define r_portc     0x08
#define r_ddrc      0x07
#define r_pinc      0x06

#define r_portd     0x0b
#define r_ddrd      0x0a
#define r_pind      0x09

/* Timer Counter 0 */
#define r_tccr0a    0x24
#define r_tccr0b    0x25
#define r_tcnt0     0x26
#define r_ocr0a     0x27
#define r_ocr0b     0x28
#define r_timsk0    0x6e
#define r_tifr0     0x15

#endif
