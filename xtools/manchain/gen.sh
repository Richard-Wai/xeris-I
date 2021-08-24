#!/bin/sh

echo ".globl rom_stand" > rom.s
echo ".globl rom_soses" >> rom.s
echo "" >> rom.s

echo ".align 2" >> rom.s
echo "rom_stand:" >> rom.s

#STD_KEYMAP
./a.out stand - 0 "0123456789ABCDEF"
#STD_CASEMAP
./a.out stand - 1 "AaZz"
#STD_XSRES
./a.out stand - 2 "^n!\`\`%\`\`/\`\`/"

./asmify stand >> rom.s
echo "" >> rom.s

#0011 FLATLINE telemetry
./a.out soses 11 0 "^nFLATLINE"
./a.out soses 11 1 "^n%ULIF \$TX/\`u10\` \$RX/\`u11\`"
./a.out soses 11 2 "^n%OMNI \$NOMINAL "
./a.out soses 11 3 "^n%OMNI \$ALTERNATE/\`u40\` "
./a.out soses 11 4 "\`u40\` "

./a.out soses 11 10 "^nreal mem = \`u40\`"

./a.out soses 11 20 "^nexec mem = \`u40\`"

./a.out soses 11 a000 "^n^lXERIS/APEX System I Release \`u40\`"
./a.out soses 11 a001 "  V\`u40\`"
./a.out soses 11 a002 ".\`u40\`"
./a.out soses 11 a003 "^nCopyright (C) \`u40\`, Richard Wai"
./a.out soses 11 a004 "^nAll rights reserved.^n^n"

#0013 SIPLEX telemetry
./a.out soses 13 0 "^n!SIPLEX/STARLINE "
./a.out soses 13 b000 "(\`x40\`/"
./a.out soses 13 b001 "\`x40\`) "
./a.out soses 13 b002 "V\`u40\`."
./a.out soses 13 b003 "\`u40\`^ninit% "
./a.out soses 13 b010 "\$lanes/\`u40\` "
./a.out soses 13 b011 "\$queues/\`u40\` "
./a.out soses 13 b020 "\$zones/\`u40\` "
./a.out soses 13 b030 "\$intz/\`u40\` "


echo ".align 2" >> rom.s
echo "rom_soses:" >> rom.s
./asmify soses >> rom.s




