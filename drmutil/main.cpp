// DRM utility
// Spiffcode Confidential
// Copyright 2003 Spiffcode, Inc. 

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <winsock2.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

// The code is derived from the Owner name. The Owner name is backed up and is the same
// after a cold boot; that's why we use it. The Code is simply a non-obvious binary
// translation of the name into a form that can be validated.

struct Code // code
{
	byte ab[4];
	byte bXor;
	byte bSum;
	byte abHash[2];
};

// ESD keys gets generated via one way translation from the code
// Retail keys are self-validating and don't require a code

struct Key // key
{
	byte ab[9];
};

dword HashBytes(byte *pb, int cb)
{
	dword dwHash = 0;
	while (cb-- != 0)
		dwHash = (dwHash << 5) + dwHash + *pb++;
	return dwHash;
}

// Returns a pseudo-random number 0 through 32767.

long glSeed;
int GetRandom()
{
    return ((glSeed = glSeed * 214013L + 2531011L) >> 16) & 0x7fff;
}

void SetRandomSeed(unsigned long nSeed)
{
	glSeed = (long)nSeed;
}

bool GetCode(char *szName, Code *pcode)
{
	if (szName == NULL || strlen(szName) == 0)
		return false;

	SetRandomSeed((long)HashBytes((byte *)szName, (int)strlen(szName)));

	// Serialize in endian independent way

	Code code;
	memset(&code, 0, sizeof(code));

	int n;
	for (n = 0; n < 4; n++) {
		code.ab[n] = (byte)GetRandom();
		code.bXor ^= code.ab[n];
		code.bSum += code.ab[n];
	}

	// Hash what we have so far - more validation

	word wHash = (word)(HashBytes(code.ab, 6) >> 16);
	code.abHash[0] = (byte)wHash;
	code.abHash[1] = (byte)(wHash >> 8);

	// Now xor the first 6 with abHash[0]

	byte *pb = (byte *)&code;
	for (n = 0; n < 6; n++)
		*pb++ ^= code.abHash[0];

	// Now xor the first 7 with abHash[1]

	pb = (byte *)&code;
	for (n = 0; n < 7; n++)
		*pb++ ^= code.abHash[1];

	// Done

	*pcode = code;
	return true;
}

bool ValidateCode(Code *pcode)
{
	Code code = *pcode;

	// Now xor the first 7 with abHash[1]

	byte *pb = (byte *)&code;
	int n;
	for (n = 0; n < 7; n++)
		*pb++ ^= code.abHash[1];

	// Now xor the first 6 with abHash[0]

	pb = (byte *)&code;
	for (n = 0; n < 6; n++)
		*pb++ ^= code.abHash[0];

	// Hash the first 6 and compare to our hash

	word wHash = (word)(HashBytes(code.ab, 6) >> 16);
	if (code.abHash[0] != (byte)wHash)
		return false;
	if (code.abHash[1] != (byte)(wHash >> 8))
		return false;

	// Xor and add the first 4

	byte bXor = 0;
	byte bSum = 0;
	for (n = 0; n < 4; n++) {
		bXor ^= code.ab[n];
		bSum += code.ab[n];
	}
	if (bXor != code.bXor)
		return false;
	if (bSum != code.bSum)
		return false;

	return true;
}

void GetKeyFromCode(Code *pcode, Key *pkey)
{
	byte *abCode = (byte *)pcode;
	int cbCode = sizeof(*pcode);

	// Generate a seed

	long lSeed = 0x29a;
	int n;
	for (n = 0; n < cbCode; n++)
		lSeed = lSeed * abCode[n] + n;

	// Initialize rotors

	byte abR1[256];
	byte abR2[256];
	byte abR3[256];

	for (n = 0; n < 256; n++)
		abR1[n] = n;
	memset(abR2, 0, sizeof(abR2));
	memset(abR3, 0, sizeof(abR3));

	for (n = 0; n < 256; n++) {
		lSeed = 3 * lSeed + abCode[n & 7];
		long lRandom = lSeed % 65521;
		int ibR1 = (lRandom & 255) % (256 - n);
		lRandom >>= 8;
		byte bTemp = abR1[255 - n];
		abR1[255 - n] = abR1[ibR1];
		abR1[ibR1] = bTemp;
		if (abR3[255 - n] == 0) {
			int nT = (lRandom & 255) % (255 - n);
			while (abR3[nT] != 0)
				nT = (nT + 1) % (255 - n);
			abR3[255 - n] = nT & 255;
			abR3[nT] = 255 - n;
		}
	}
	for (n = 0; n < 256; n++)
		abR2[abR1[n]] = n;

	// Encrypt the code into a key

	int nT = 0;
	byte *abKey = (byte *)pkey;
	int cbKey = sizeof(*pkey);
	for (n = 0; n < cbKey; n++) {
		int nSlotR1 = abR1[(n + nT) & 255];
		int nSlotR3 = abR3[nSlotR1 & 255];
		int nSlotR2 = abR2[nSlotR3 & 255];
		abKey[n] = abR2[nSlotR2 & 255] - nT;
		nT = (nT + 1) & 255;
	}
}

void FillTemplate(char *psz, char *pszFormat, byte *pb)
{
	char *pchSrc = pszFormat;
	char *pchDst = psz;

	bool fMSB = true;
	byte bT = 0;
	int cb = (int)strlen(pszFormat) + 1;
	while (cb-- != 0) {
		if (*pchSrc != '?') {
			*pchDst++ = *pchSrc++;
			continue;
		}

		int nNibble = 0;
		if (fMSB) {
			fMSB = false;
			bT = *pb++;
			nNibble = (bT >> 4) & 0x0f;
		} else {
			fMSB = true;
			nNibble = (bT & 0x0f);
		}

		char chT;
		if (nNibble < 0x0a) {
			chT = '0' + nNibble;
		} else {
			chT = 'A' + nNibble - 0x0a;
		}

		*pchDst++ = chT;
		pchSrc++;
	}
}

bool ParseCode(char *psz, Code *pcode)
{
	int an[4];
	int c = sscanf(psz, "C-%04x-%04x-%04x-%04x", &an[0], &an[1], &an[2], &an[3]);
	if (c != 4)
		return false;


	byte *pb = (byte *)pcode;
	for (int i = 0; i < 4; i++) {
		for (int j = 1; j >= 0; j--) {
			*pb++ = (byte)(an[i] >> (j * 8)) & 0xff;
		}
	}

	return true;
}

bool ParseKey(char *psz, Key *pkey)
{
	int an[3];
	int c = sscanf(psz, "K-%06x-%06x-%06x", &an[0], &an[1], &an[2]);
	if (c != 3)
		return false;

	byte *pb = (byte *)pkey;
	for (int i = 0; i < 3; i++) {
		for (int j = 2; j >= 0; j--) {
			*pb++ = (byte)(an[i] >> (j * 8)) & 0xff;
		}
	}

	return true;
}

void Usage()
{
	printf("\n");
	printf("Usage:\n");
	printf("\n");
	printf("drmutil key code\t\tGenerate a key given a code\n");
	printf("drmutil name \"name\"\t\tGenerate code and key given a name\n");
	printf("drmutil valid code\t\tCheck if a code is valid\n");
	printf("drmutil retailkey number\tGenerate retail key. Number range 101-16777215\n");
	printf("drmutil retailkeyvalid key\tCheck if a retail key is valid\n");
	printf("drmutil pc\t\t\tPrint codes for this desktop PC\n");
	printf("\n");
	printf("Code format:\tC-XXXX-XXXX-XXXX-XXXX\n");
	printf("Key format:\tK-XXXXXX-XXXXXX-XXXXXX\n");
	printf("'X' is a digit 0-9,A-F\n");
	exit(1);
}

void OutputCodeAndKey(char *sz)
{
	// Gen / output code

	Code code;
	if (!GetCode(sz, &code)) {
		printf("Can't generate code; invalid name!\n");
		Usage();
	}
	printf("Code and key for \"%s\":\n", sz);
	FillTemplate(sz, "C-\?\?\?\?-\?\?\?\?-\?\?\?\?-\?\?\?\?", (byte *)&code);
	printf("%s\n", sz);

	// Gen / output key

	Key key;
	GetKeyFromCode(&code, &key);
	FillTemplate(sz, "K-\?\?\?\?\?\?-\?\?\?\?\?\?-\?\?\?\?\?\?", (byte *)&key);
	printf("%s\n", sz);
}

void GetRetailKey(int nNumber, Key *pkey)
{
	SetRandomSeed(nNumber);
	char szT[4];
	szT[0] = (byte)GetRandom();
	szT[1] = (byte)GetRandom();
	szT[2] = (byte)GetRandom();
	szT[3] = 0;

	// Ensure none of these have zeros in them

	for (int n = 0; n < sizeof(szT) - 1; n++) {
		while (szT[n] == 0)
			szT[n] = (byte)GetRandom();
	}

	// Get code and key

	Code code;
	GetCode(szT, &code);
	Key key;
	GetKeyFromCode(&code, &key);

	// Now fill in the first 3 bytes with the encoded "name"

	key.ab[0] = szT[0];
	SetRandomSeed((dword)szT[0]);
	key.ab[1] = szT[1] ^ (byte)GetRandom();
	key.ab[2] = szT[2] ^ (byte)GetRandom();
	*pkey = key;
}

void OutputRetailKey(char *sz)
{
	// Number can be between 101 and 2^24-1 (3 bytes)

	int nNumber = atoi(sz);
	if (nNumber < 101 || nNumber > 16777215) {
		printf("Seed number is invalid\n");
		Usage();
	}

	// Generate retail key

	Key key;
	GetRetailKey(nNumber, &key);

	// Output

	printf("Seed: %d\n", nNumber);
	char szKey[64];
	FillTemplate(szKey, "K-\?\?\?\?\?\?-\?\?\?\?\?\?-\?\?\?\?\?\?", (byte *)&key);
	printf("%s\n", szKey);
}

bool CompareKeys(Key *pkeyA, Key *pkeyB)
{
	byte *pbA = (byte *)pkeyA;
	byte *pbB = (byte *)pkeyB;
	int cb = sizeof(*pkeyA);
	while (cb-- != 0) {
		if (*pbA++ != *pbB++)
			return false;
	}
	return true;
}

bool ValidateRetailKey(Key *pkey)
{
	char szT[4];

	// Derive the "name" from the key

	szT[0] = pkey->ab[0];
	SetRandomSeed((dword)szT[0]);
	szT[1] = pkey->ab[1] ^ (byte)GetRandom();
	szT[2] = pkey->ab[2] ^ (byte)GetRandom();
	szT[3] = 0;

	// Fill in zeros

	int n;
	for (n = 0; n < sizeof(szT) - 1; n++) {
		while (szT[n] == 0)
			szT[n] = (byte)GetRandom();
	}

	// Get code

	Code code;
	GetCode(szT, &code);

	// Gen full key from this

	Key keyT;
	GetKeyFromCode(&code, &keyT);

	// Compare keys - compares the last 6 bytes because we copy over the first 3

	Key keyReconstructed = *pkey;
	for (n = 0; n < sizeof(szT) - 1; n++)
		keyReconstructed.ab[n] = keyT.ab[n];

	// Now compare these two keys

	return CompareKeys(&keyT, &keyReconstructed);
}

int main(int argc, char **argv)
{
	char *pszCommand = argv[1];
	char *pszArg = argv[2];

	if (argc == 2) {
		if (strcmp(pszCommand, "pc") == 0) {
			char sz[64];
			sz[0] = 0;

			// Get host name

			WSADATA wsad;
			WSAStartup(MAKEWORD(1, 1), &wsad);
			gethostname(sz, sizeof(sz) - 1);
			WSACleanup();

			OutputCodeAndKey(sz);
			return 0;
		}
		Usage();
	}

	if (argc == 3) {
		if (strcmp(pszCommand, "key") == 0) {
			Code code;
			if (!ParseCode(pszArg, &code)) {
				printf("Code entered in incorrect format!\n");
				Usage();
			}
			if (!ValidateCode(&code)) {
				printf("This code is invalid. It fails internal consistency check\n");
				Usage();
			}
			Key key;
			GetKeyFromCode(&code, &key);
			char szKey[64];
			FillTemplate(szKey, "K-\?\?\?\?\?\?-\?\?\?\?\?\?-\?\?\?\?\?\?", (byte *)&key);
			printf("%s\n", szKey);
			return 0;
		}

		if (strcmp(pszCommand, "name") == 0) {
			OutputCodeAndKey(pszArg);
			return 0;
		}

		if (strcmp(pszCommand, "valid") == 0) {
			Code code;
			if (!ParseCode(pszArg, &code)) {
				printf("Code entered in incorrect format!\n");
				Usage();
			}
			if (!ValidateCode(&code)) {
				printf("Invalid code\n");
				return 1;
			}

			printf("Valid code\n");
			return 0;
		}

		if (strcmp(pszCommand, "retailkey") == 0) {
			OutputRetailKey(pszArg);
			return 0;
		}

		if (strcmp(pszCommand, "retailkeyvalid") == 0) {
			Key key;
			if (!ParseKey(pszArg, &key)) {
				printf("Key entered in incorrect format!\n");
				Usage();
			}
			if (!ValidateRetailKey(&key)) {
				printf("Retail key invalid.\n");
			} else {
				printf("Retail key valid.\n");
			}
			return 0;
		}
	}

	Usage();
}

