// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "testrunner.h"


bool isMul(uint16_t opcode)
{
    bool result = false;

    if ((opcode & 0b1111000111000000) == 0b1100000111000000) result = true;
    if ((opcode & 0b1111000111000000) == 0b1100000011000000) result = true;

    return result;
}

bool isDiv(uint16_t opcode)
{
    if ((opcode & 0b1111000111000000) == 0b1000000111000000) return true;
    if ((opcode & 0b1111000111000000) == 0b1000000011000000) return true;
    return false;
}

bool isMulOrDiv(uint16_t opcode)
{
    return isDiv(opcode) || isMul(opcode);
}

bool isAbcd(uint16_t opcode)
{
    if ((opcode & 0b1111000111111000) == 0b1000000100000000) return true;
    if ((opcode & 0b1111000111111000) == 0b1000000100001000) return true;
    return false;
}

bool isSbcd(uint16_t opcode)
{
    if ((opcode & 0b1111000111111000) == 0b1100000100000000) return true;
    if ((opcode & 0b1111000111111000) == 0b1100000100001000) return true;
    return false;
}

bool isNbcd(uint16_t opcode)
{
    if ((opcode & 0b1111111111000000) == 0b0100100000000000) return true;
    return false;
}

bool isBcd(uint16_t opcode)
{
    return isAbcd(opcode) || isSbcd(opcode) || isNbcd(opcode);
}

uint32 smartRandom()
{
    switch (rand() % 16) {

        case  0: // 8-bit numbers
        case  1: return rand() & 0xFF;
        case  2: return 0x80;
        case  3: return 0xFF;

        case  4: // 16-bit numbers
        case  5: return rand() & 0xFFFF;
        case  6: return 0x8000;
        case  7: return 0xFFFF;

        case  8: // 32-bit numbers
        case  9: return rand() & 0xFFFFFFFF;
        case 10: return 0x80000000;
        case 11: return 0xFFFFFFFF;

        default: return 0;
    }
}

void setupMusashi()
{
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
}

void setupMoira()
{

}

void createTestCase(Setup &s)
{
    s.sr = 0;

    for (int i = 0; i < 8; i++) s.d[i] = smartRandom();
    for (int i = 0; i < 8; i++) s.a[i] = smartRandom();

    for (unsigned i = 0; i < sizeof(s.mem); i++) {
        s.mem[i] = smartRandom();
    }
}

void setupInstruction(Setup &s, uint32_t pc, uint16_t opcode)
{
    s.pc = pc;
    s.opcode = opcode;

    s.mem[pc] = opcode >> 8;
    s.mem[pc + 1] = opcode & 0xFF;
}

void resetMusashi(Setup &s)
{
    m68ki_set_sr_noint(0);

    m68k_pulse_reset();

    memcpy(mem, s.mem, sizeof(mem));

    m68k_set_reg(M68K_REG_USP, 0);
    m68k_set_reg(M68K_REG_ISP, 0);
    m68k_set_reg(M68K_REG_MSP, 0);
    // m68ki_set_sr_noint(s.sr);

    for (int i = 0; i < 8; i++) {
        m68k_set_reg((m68k_register_t)(M68K_REG_D0 + i), s.d[i]);
        m68k_set_reg((m68k_register_t)(M68K_REG_A0 + i), s.a[i]);
    }
}

void resetMoira(Setup &s)
{
    moiracpu->reset();

    memcpy(mem, s.mem, sizeof(mem));

    // moiracpu->setSR(s.sr);

    for (int i = 0; i < 8; i++) {
        moiracpu->writeD(i,s.d[i]);
        moiracpu->writeA(i,s.a[i]);
    }
}

void run()
{
    Setup setup;

    printf("Moira CPU tester. (C) Dirk W. Hoffmann, 2019\n\n");
    printf("The test program runs Moira agains Musashi with randomly generated data.\n");
    printf("It runs until a bug has been found.\n\n");

    setupMusashi();
    setupMoira();
    srand((int)time(NULL));

    for (long round = 1 ;; round++) {

        createTestCase(setup);

        printf("Round %ld ", round); fflush(stdout);
        clock_t start = clock();

        // Iterate through all opcodes
        for (int opcode = 0x0000; opcode < 65536; opcode++) {

            if ((opcode & 0xFFF) == 0) { printf("."); fflush(stdout); }

            setupInstruction(setup, pc, opcode);
            runSingleTest(setup);
        }

        clock_t now = clock();
        double elapsed = (double(now - start) / double(CLOCKS_PER_SEC));
        printf(" PASSED (%.2f sec)\n", elapsed);
    }
}

void runSingleTest(Setup &s)
{
    Result mur, mor;

    if (TEST_DASM) {

        int muc, moc;
        char mus[128], mos[128];

        muc = m68k_disassemble(mus, s.pc, M68K_CPU_TYPE_68000);
        moc = moiracpu->disassemble(s.pc, mos);
        compare(muc, moc, mus, mos);
    }

    // Skip illegal instructions
    if (moiracpu->isIllegalInstr(s.opcode)) return;
    if (moiracpu->isLineAInstr(s.opcode)) return;
    if (moiracpu->isLineFInstr(s.opcode)) return;

    // Skip NBCD, SBCD, ABCD (likely to be broken in Musashi)
    if (isBcd(s.opcode)) return;

    if (VERBOSE) {
         char mos[128];
         moiracpu->disassemble(s.pc, mos);
         printf("$%04x: %s\n", s.opcode, mos);
     }

    // Reset the sandbox (memory accesses observer)
    moiracpu->sandbox.prepare();

    // Prepare Musashi
    resetMusashi(s);

    // Run Musashi
    mur.cycles = m68k_execute(1);

    // Record the result
    mur.pc = m68k_get_reg(NULL, M68K_REG_PC);
    mur.sr = m68k_get_reg(NULL, M68K_REG_SR);
    mur.usp = m68k_get_reg(NULL, M68K_REG_USP);
    mur.ssp = m68k_get_reg(NULL, M68K_REG_ISP);
    for (int i = 0; i < 8; i++) {
        mur.d[i] = m68k_get_reg(NULL, (m68k_register_t)(M68K_REG_D0 + i));
        mur.a[i] = m68k_get_reg(NULL, (m68k_register_t)(M68K_REG_A0 + i));
    }

    // Prepare Moira
    resetMoira(s);

    // Run Moira
    int64_t clock = moiracpu->getClock();
    moiracpu->process();

    // Record the result
    mor.cycles = (int)(moiracpu->getClock() - clock);
    mor.pc = moiracpu->getPC();
    mor.sr = moiracpu->getSR();
    for (int i = 0; i < 8; i++) mor.d[i] = moiracpu->readD(i);
    for (int i = 0; i < 8; i++) mor.a[i] = moiracpu->readA(i);

    // Compare
    compare(s, mur, mor);
}

void dumpSetup(Setup &s)
{
    printf("PC: %4x ", s.pc);
    printf("SR: %2x\n", s.sr);
    printf("         Dn: ");
    for (int i = 0; i < 8; i++) printf("%8x ", s.d[i]);
    printf("\n");
    printf("         An: ");
    for (int i = 0; i < 8; i++) printf("%8x ", s.a[i]);
    printf("\n\n");
}

void dumpResult(Result &r)
{
    printf("PC: %4x ", r.pc);
    printf("SR: %2x ", r.sr);
    printf("Elapsed cycles: %d\n" , r.cycles);

    printf("         Dn: ");
    for (int i = 0; i < 8; i++) printf("%8x ", r.d[i]);
    printf("\n");
    printf("         An: ");
    for (int i = 0; i < 8; i++) printf("%8x ", r.a[i]);
    printf("\n\n");
}

void compare(Setup &s, Result &r1, Result &r2)
{
    bool error = false;
    char str[128];

    error |= (r1.pc != r2.pc);
    error |= (r1.sr != r2.sr);
    error |= !isMulOrDiv(s.opcode) && (r1.cycles != r2.cycles);
    for (int i = 0; i < 8; i++) error |= r1.d[i] != r2.d[i];
    for (int i = 0; i < 8; i++) error |= r1.a[i] != r2.a[i];

    if (error) {

        moiracpu->disassemble(s.pc, str);
        printf("\nMISMATCH FOUND (opcode $%x out of $FFFF):\n\n", s.opcode);
        printf("Instruction: %s\n\n", str);

        printf("Setup:   ");
        dumpSetup(s);

        printf("Musashi: ");
        dumpResult(r1);

        printf("Moira:   ");
        dumpResult(r2);

        bugReport();
    }
}

void compare(int c1, int c2, char *s1, char *s2)
{
    if (c1 != c2 || strcmp(s1, s2) != 0) {

        printf("\nDISASSEMBLER MISMATCH FOUND:\n\n");
        printf("    Musashi: %s\n", s1);
        printf("      Moira: %s\n", s2);
        printf("      Bytes: %d / %d\n\n", c1, c2);

        bugReport();
    }
}

void bugReport()
{
    printf("Please send a bug report to: dirk.hoffmann@me.com\n");
    printf("Thanks you!\n\n");
    assert(false);
}
