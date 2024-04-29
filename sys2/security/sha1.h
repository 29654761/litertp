/**
 * @file sha1.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once
#ifndef ___SHA1_HDR___
#define ___SHA1_HDR___

#define _CRT_SECURE_NO_WARNINGS

#if !defined(SHA1_UTILITY_FUNCTIONS) && !defined(SHA1_NO_UTILITY_FUNCTIONS)
#define SHA1_UTILITY_FUNCTIONS
#endif

#include <memory.h> // Needed for memset and memcpy

#ifdef SHA1_UTILITY_FUNCTIONS
#include <stdio.h>  // Needed for file access and sprintf
#include <string.h> // Needed for strcat and strcpy
#endif

#ifdef _MSC_VER
#include <stdlib.h>
#endif

// You can define the endian mode in your files, without modifying the SHA1
// source files. Just #define SHA1_LITTLE_ENDIAN or #define SHA1_BIG_ENDIAN
// in your files, before including the SHA1.h header file. If you don't
// define anything, the class defaults to little endian.

#if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
#define SHA1_LITTLE_ENDIAN
#endif

// Same here. If you want variable wiping, #define SHA1_WIPE_VARIABLES, if
// not, #define SHA1_NO_WIPE_VARIABLES. If you don't define anything, it
// defaults to wiping.

#if !defined(SHA1_WIPE_VARIABLES) && !defined(SHA1_NO_WIPE_VARIABLES)
#define SHA1_WIPE_VARIABLES
#endif

/////////////////////////////////////////////////////////////////////////////
// Define 8- and 32-bit variables

#ifndef UINT_32

#ifdef _MSC_VER

#define UINT_8  unsigned __int8
#define UINT_32 unsigned __int32

#else

#define UINT_8 unsigned char

#if (ULONG_MAX == 0xFFFFFFFF)
#define UINT_32 unsigned long
#else
#define UINT_32 unsigned int
#endif

#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Declare SHA1 workspace

typedef union
{
    UINT_8  c[64];
    UINT_32 l[16];
} SHA1_WORKSPACE_BLOCK;

class sha1
{
public:
#ifdef SHA1_UTILITY_FUNCTIONS
    // Two different formats for ReportHash(...)
    enum
    {
        REPORT_HEX = 0,
        REPORT_DIGIT = 1
    };
#endif

    // Constructor and Destructor
    sha1();
    ~sha1();

    UINT_32 m_state[5];
    UINT_32 m_count[2];
    UINT_32 __reserved1[1];
    UINT_8  m_buffer[64];
    UINT_8  m_digest[20];
    UINT_32 __reserved2[3];

    void reset();

    // Update the hash value
    void update(UINT_8* data, UINT_32 len);
#ifdef SHA1_UTILITY_FUNCTIONS
    bool hash_file(char* szFileName);
#endif

    // Finalize hash and report
    void final();

    // Report functions: as pre-formatted and raw data
#ifdef SHA1_UTILITY_FUNCTIONS
    void report_hash(char* szReport, unsigned char uReportType = REPORT_HEX);
#endif
    void get_hash(UINT_8* puDest);

private:
    // Private SHA-1 transformation
    void transform(UINT_32* state, UINT_8* buffer);

    // Member variables
    UINT_8 m_workspace[64];
    SHA1_WORKSPACE_BLOCK* m_block; // SHA1 pointer to the byte array above
};

#endif