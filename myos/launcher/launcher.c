/* ================================================================
 *  MyOS Boot Launcher - Windows EXE
 *  Redémarre le PC et boot directement sur MyOS (clé USB)
 *  Via les variables UEFI BootNext
 * ================================================================ */

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
#include <string.h>

/* ---- UEFI Global Variable GUID ---- */
#define EFI_GLOBAL_GUID L"{8be4df61-93ca-11d2-aa0d-00e098032b8c}"

/* ---- EFI Device Path Node ---- */
#pragma pack(1)
typedef struct {
    uint8_t  Type;
    uint8_t  SubType;
    uint16_t Length;
} DP_NODE;

typedef struct {
    uint32_t Attributes;
    uint16_t FilePathListLength;
    /* suivi de: wchar_t Description[] + DP_NODE FilePathList[] */
} EFI_LOAD_OPTION;
#pragma pack()

/* ====================================================
 * Couleurs console
 * ==================================================== */
static void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
#define COL_WHITE   15
#define COL_CYAN    11
#define COL_GREEN   10
#define COL_YELLOW  14
#define COL_RED     12
#define COL_GREY    8

/* ====================================================
 * Activer le privilège UEFI (SeSystemEnvironmentPrivilege)
 * ==================================================== */
static BOOL EnableEFIPrivilege(void) {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    if (!LookupPrivilegeValueW(NULL, L"SeSystemEnvironmentPrivilege", &luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    DWORD err = GetLastError();
    CloseHandle(hToken);
    return (err == ERROR_SUCCESS);
}

/* ====================================================
 * Vérifier si l'entrée de boot contient un device USB
 * Type=0x03 (Messaging), SubType=0x05 (USB)
 * ==================================================== */
static BOOL HasUSBDevicePath(uint8_t* data, DWORD size) {
    if (size < sizeof(EFI_LOAD_OPTION)) return FALSE;

    EFI_LOAD_OPTION* opt = (EFI_LOAD_OPTION*)data;
    uint32_t offset = sizeof(EFI_LOAD_OPTION);

    /* Sauter la description (wchar_t null-terminated) */
    while (offset + 1 < size) {
        wchar_t c = *(wchar_t*)(data + offset);
        offset += 2;
        if (c == 0) break;
    }

    /* Parcourir les nœuds du device path */
    uint32_t dp_end = offset + opt->FilePathListLength;
    if (dp_end > size) dp_end = size;

    while (offset + sizeof(DP_NODE) <= dp_end) {
        DP_NODE* node = (DP_NODE*)(data + offset);
        if (node->Length < sizeof(DP_NODE)) break;

        /* USB: Type=3 SubType=5 */
        if (node->Type == 0x03 && node->SubType == 0x05)
            return TRUE;
        /* USB Class: Type=3 SubType=0x0F */
        if (node->Type == 0x03 && node->SubType == 0x0F)
            return TRUE;
        /* End of Hardware Device Path */
        if (node->Type == 0x7F && node->SubType == 0xFF)
            break;

        offset += node->Length;
    }
    return FALSE;
}

/* ====================================================
 * Récupérer la description d'une entrée de boot
 * ==================================================== */
static void GetEntryDescription(uint8_t* data, DWORD size,
                                 wchar_t* desc, int maxLen) {
    if (size < sizeof(EFI_LOAD_OPTION)) { desc[0] = 0; return; }

    uint32_t offset = sizeof(EFI_LOAD_OPTION);
    int i = 0;

    while (offset + 1 < size && i < maxLen - 1) {
        wchar_t c = *(wchar_t*)(data + offset);
        offset += 2;
        if (c == 0) break;
        desc[i++] = c;
    }
    desc[i] = 0;
}

/* ====================================================
 * Vérifier si la description ressemble à USB/Amovible
 * ==================================================== */
static BOOL DescriptionIsUSB(const wchar_t* desc) {
    wchar_t low[256];
    int i;
    for (i = 0; desc[i] && i < 255; i++)
        low[i] = (desc[i] >= L'A' && desc[i] <= L'Z') ? desc[i] + 32 : desc[i];
    low[i] = 0;

    return (wcsstr(low, L"usb")        != NULL ||
            wcsstr(low, L"removable")  != NULL ||
            wcsstr(low, L"sandisk")    != NULL ||
            wcsstr(low, L"kingston")   != NULL ||
            wcsstr(low, L"myos")       != NULL ||
            wcsstr(low, L"verbatim")   != NULL ||
            wcsstr(low, L"corsair")    != NULL ||
            wcsstr(low, L"toshiba")    != NULL ||
            wcsstr(low, L"cruzer")     != NULL ||
            wcsstr(low, L"datatraveler") != NULL);
}

/* ====================================================
 * Redémarrer le système
 * ==================================================== */
static void DoReboot(void) {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    OpenProcessToken(GetCurrentProcess(),
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
    LookupPrivilegeValueW(NULL, SE_SHUTDOWN_NAME, &luid);
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(hToken);

    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER);
}

/* ====================================================
 * Afficher un header ASCII
 * ==================================================== */
static void PrintHeader(void) {
    SetColor(COL_CYAN);
    wprintf(L"\n");
    wprintf(L"  ╔══════════════════════════════════════════╗\n");
    wprintf(L"  ║       MyOS Boot Launcher v1.0            ║\n");
    wprintf(L"  ║   Redémarre directement sur MyOS USB     ║\n");
    wprintf(L"  ╚══════════════════════════════════════════╝\n\n");
    SetColor(COL_WHITE);
}

/* ====================================================
 * Programme principal
 * ==================================================== */
int wmain(void) {
    /* Initialiser la console */
    SetConsoleOutputCP(65001);
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hCon, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    PrintHeader();

    /* ---- Vérifier les droits admin ---- */
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuth, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0,0,0,0,0,0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    if (!isAdmin) {
        SetColor(COL_RED);
        wprintf(L"  [ERREUR] Ce programme necessite les droits Administrateur.\n");
        wprintf(L"  Clic droit sur le .exe -> 'Executer en tant qu administrateur'\n\n");
        SetColor(COL_WHITE);
        wprintf(L"  Appuyez sur Entree pour quitter...\n");
        getchar();
        return 1;
    }

    SetColor(COL_GREEN);
    wprintf(L"  [OK] Droits administrateur detectes\n");
    SetColor(COL_WHITE);

    /* ---- Activer le privilège UEFI ---- */
    if (!EnableEFIPrivilege()) {
        SetColor(COL_YELLOW);
        wprintf(L"  [AVERT] Privilege UEFI non disponible (system Legacy BIOS?)\n");
        SetColor(COL_WHITE);
    } else {
        SetColor(COL_GREEN);
        wprintf(L"  [OK] Privilege UEFI active\n");
        SetColor(COL_WHITE);
    }

    /* ---- Lire BootOrder ---- */
    wprintf(L"\n  Scan des entrees de boot UEFI...\n\n");

    uint8_t bootOrderBuf[512];
    DWORD   bootOrderSize = sizeof(bootOrderBuf);

    if (!GetFirmwareEnvironmentVariableW(L"BootOrder", EFI_GLOBAL_GUID,
                                          bootOrderBuf, bootOrderSize)) {
        DWORD err = GetLastError();
        SetColor(COL_RED);
        wprintf(L"  [ERREUR] Impossible de lire BootOrder (erreur %lu)\n", err);
        if (err == 1314) {
            wprintf(L"  -> Privilege insuffisant. Verifie que Secure Boot est desactive.\n");
        } else if (err == 998 || err == 1) {
            wprintf(L"  -> Ce PC utilise peut-etre le mode BIOS Legacy (non UEFI).\n");
            wprintf(L"  -> Dans ce cas, selectionne la cle USB dans le boot menu.\n");
        }
        SetColor(COL_WHITE);
        wprintf(L"\n  Tentative de redemarrage vers le firmware (F12)...\n");
        Sleep(2000);

        /* Fallback: reboot vers firmware UEFI */
        HANDLE hToken2;
        TOKEN_PRIVILEGES tp2;
        LUID luid2;
        OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken2);
        LookupPrivilegeValueW(NULL, SE_SHUTDOWN_NAME, &luid2);
        tp2.PrivilegeCount           = 1;
        tp2.Privileges[0].Luid       = luid2;
        tp2.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken2, FALSE, &tp2, sizeof(tp2), NULL, NULL);
        CloseHandle(hToken2);

        /* Reboot vers UEFI firmware interface */
        InitiateShutdownW(NULL, NULL, 0,
            SHUTDOWN_RESTART | SHUTDOWN_FORCE_OTHERS |
            SHUTDOWN_RESTARTAPPS | 0x00400000 /* SHUTDOWN_BOOT_TO_FW */,
            SHTDN_REASON_MAJOR_OTHER);

        Sleep(3000);
        DoReboot();
        return 1;
    }

    /* Calculer le nombre d'entrées */
    DWORD entryCount = bootOrderSize / 2;
    uint16_t* bootOrder = (uint16_t*)bootOrderBuf;

    /* ---- Scanner chaque entrée ---- */
    uint16_t  usbEntries[32];
    wchar_t   usbDescs[32][256];
    int       usbCount = 0;

    for (DWORD i = 0; i < entryCount && usbCount < 32; i++) {
        wchar_t varName[16];
        swprintf(varName, 16, L"Boot%04X", bootOrder[i]);

        uint8_t buf[4096];
        DWORD   sz = sizeof(buf);

        if (!GetFirmwareEnvironmentVariableW(varName, EFI_GLOBAL_GUID, buf, sz))
            continue;

        /* Obtenir la description */
        wchar_t desc[256];
        GetEntryDescription(buf, sz, desc, 256);

        SetColor(COL_GREY);
        wprintf(L"  Boot%04X: %ls\n", bootOrder[i], desc);
        SetColor(COL_WHITE);

        /* Vérifier si c'est une entrée USB */
        if (HasUSBDevicePath(buf, sz) || DescriptionIsUSB(desc)) {
            usbEntries[usbCount] = bootOrder[i];
            wcsncpy(usbDescs[usbCount], desc, 255);
            usbDescs[usbCount][255] = 0;
            usbCount++;
        }
    }

    wprintf(L"\n");

    /* ---- Aucune entrée USB trouvée ---- */
    if (usbCount == 0) {
        SetColor(COL_RED);
        wprintf(L"  [ERREUR] Aucune cle USB bootable detectee !\n\n");
        SetColor(COL_YELLOW);
        wprintf(L"  Assure-toi que :\n");
        wprintf(L"    1. La cle USB MyOS est bien branchee\n");
        wprintf(L"    2. Elle a ete gravee avec Rufus\n");
        wprintf(L"    3. Secure Boot est desactive dans le BIOS\n\n");
        SetColor(COL_WHITE);
        wprintf(L"  Appuyez sur Entree pour quitter...\n");
        getchar();
        return 1;
    }

    /* ---- Choisir l'entrée (si plusieurs) ---- */
    int chosen = 0;
    if (usbCount > 1) {
        SetColor(COL_CYAN);
        wprintf(L"  Plusieurs peripheriques USB detectes :\n\n");
        SetColor(COL_WHITE);
        for (int i = 0; i < usbCount; i++) {
            wprintf(L"    [%d] %ls\n", i + 1, usbDescs[i]);
        }
        wprintf(L"\n  Choix (1-%d) : ", usbCount);
        int choice = 0;
        wscanf(L"%d", &choice);
        if (choice >= 1 && choice <= usbCount)
            chosen = choice - 1;
    }

    /* ---- Afficher le choix ---- */
    SetColor(COL_GREEN);
    wprintf(L"  [OK] Cible selectionnee : %ls\n", usbDescs[chosen]);
    SetColor(COL_WHITE);

    /* ---- Définir BootNext ---- */
    uint16_t bootNext = usbEntries[chosen];

    if (!SetFirmwareEnvironmentVariableW(L"BootNext", EFI_GLOBAL_GUID,
                                          &bootNext, sizeof(bootNext))) {
        DWORD err = GetLastError();
        SetColor(COL_RED);
        wprintf(L"  [ERREUR] Impossible de definir BootNext (erreur %lu)\n\n", err);
        SetColor(COL_YELLOW);
        wprintf(L"  -> Desactive Secure Boot dans le BIOS et reessaie.\n");
        SetColor(COL_WHITE);
        wprintf(L"\n  Appuyez sur Entree pour quitter...\n");
        getchar();
        return 1;
    }

    SetColor(COL_GREEN);
    wprintf(L"  [OK] BootNext configure : Boot%04X\n", bootNext);
    SetColor(COL_WHITE);

    /* ---- Compte à rebours avant reboot ---- */
    wprintf(L"\n");
    SetColor(COL_CYAN);
    wprintf(L"  ════════════════════════════════════════════\n");
    wprintf(L"   Redemarrage vers MyOS dans 3 secondes...\n");
    wprintf(L"  ════════════════════════════════════════════\n\n");
    SetColor(COL_WHITE);

    for (int i = 3; i > 0; i--) {
        SetColor(COL_YELLOW);
        wprintf(L"    %d...\n", i);
        SetColor(COL_WHITE);
        Sleep(1000);
    }

    wprintf(L"\n  Redemarrage !\n\n");
    Sleep(500);

    DoReboot();
    return 0;
}
