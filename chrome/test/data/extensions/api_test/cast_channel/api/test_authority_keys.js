// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var VALID_SIGNATURE = [
    0xA3, 0xCD, 0xE8, 0x92, 0xED, 0xE3, 0x3F, 0xEC, 0x63, 0x4F, 0x2B, 0x0D,
    0x05, 0xA9, 0x6A, 0xA7, 0xF6, 0x5F, 0x25, 0x13, 0xCB, 0xB7, 0xC4, 0x7E,
    0x21, 0x48, 0x59, 0x31, 0xEA, 0x5B, 0x01, 0x11, 0x52, 0x45, 0x58, 0xB3,
    0x2E, 0x87, 0x85, 0x5B, 0x95, 0xF2, 0x53, 0x07, 0xBA, 0x95, 0x06, 0x82,
    0xF8, 0xF1, 0x45, 0x40, 0x74, 0x99, 0x78, 0x66, 0x72, 0x79, 0xE1, 0x91,
    0xA7, 0x00, 0x89, 0xFE, 0x57, 0x9F, 0xD3, 0x18, 0x7A, 0x49, 0x62, 0x65,
    0x54, 0xC5, 0x8D, 0x4E, 0x4E, 0x7E, 0xCF, 0x45, 0x3C, 0xE0, 0x64, 0x1D,
    0x8B, 0x32, 0x4F, 0xFC, 0x24, 0xC2, 0x78, 0x7A, 0x48, 0xC0, 0xCA, 0x1B,
    0x37, 0xE9, 0x32, 0x0D, 0xA8, 0xE0, 0x3B, 0x27, 0xC3, 0x0C, 0xB8, 0x64,
    0xB5, 0xCE, 0x8B, 0x1E, 0x64, 0xA5, 0x14, 0x61, 0xBA, 0x27, 0xB0, 0x7C,
    0x5B, 0xAE, 0xE8, 0xCE, 0x24, 0xBC, 0x64, 0x51, 0x1D, 0xAD, 0xC4, 0xFD,
    0x59, 0x25, 0xC5, 0xAE, 0x32, 0xD5, 0xC9, 0x44, 0x47, 0x8E, 0xF8, 0x9D,
    0x64, 0x3E, 0x62, 0x16, 0xDB, 0x88, 0x98, 0x8E, 0xA4, 0xF5, 0x1F, 0x4E,
    0xA1, 0x72, 0xD1, 0xE9, 0x2C, 0x80, 0xC2, 0x98, 0xDD, 0xAC, 0xBF, 0xAF,
    0x94, 0x73, 0x24, 0xDC, 0x6A, 0x25, 0xE8, 0xA4, 0x3D, 0xF1, 0x8A, 0x45,
    0xD2, 0x08, 0x47, 0x9C, 0xDD, 0xFF, 0x14, 0x55, 0xB8, 0x89, 0xBB, 0xC8,
    0xC2, 0x39, 0x91, 0x6E, 0x9F, 0xAD, 0x51, 0x5D, 0xE1, 0xF1, 0x34, 0x5C,
    0x08, 0xE8, 0xE5, 0x5D, 0x51, 0xA9, 0x6F, 0x33, 0xED, 0x35, 0x14, 0x65,
    0x4E, 0xA7, 0xEF, 0xED, 0x1C, 0x66, 0xE5, 0x85, 0xE4, 0x64, 0xC2, 0xE6,
    0x24, 0x93, 0x19, 0x89, 0x3B, 0x39, 0xFA, 0x8F, 0x05, 0x31, 0x13, 0x13,
    0x41, 0x94, 0xF8, 0x85, 0x14, 0x77, 0x0D, 0x0F, 0x80, 0xFE, 0x3A, 0x1F,
    0x56, 0x2C, 0x93, 0x18
];

var VALID_AUTHORITY_KEYS = [
    0x0A, 0xB3, 0x02, 0x0A, 0x20, 0x52, 0x9D, 0x9C, 0xD6, 0x7F, 0xE5, 0xEB,
    0x69, 0x8E, 0x70, 0xDD, 0x26, 0xD7, 0xD8, 0xF1, 0x26, 0x59, 0xF1, 0xE6,
    0xE5, 0x23, 0x48, 0xBF, 0x6A, 0x5C, 0xF7, 0x16, 0xE1, 0x3F, 0x41, 0x0E,
    0x73, 0x12, 0x8E, 0x02, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01,
    0x00, 0xBC, 0x22, 0x80, 0xBD, 0x80, 0xF6, 0x3A, 0x21, 0x00, 0x3B, 0xAE,
    0x76, 0x5E, 0x35, 0x7F, 0x3D, 0xC3, 0x64, 0x5C, 0x55, 0x94, 0x86, 0x34,
    0x2F, 0x05, 0x87, 0x28, 0xCD, 0xF7, 0x69, 0x8C, 0x17, 0xB3, 0x50, 0xA7,
    0xB8, 0x82, 0xFA, 0xDF, 0xC7, 0x43, 0x2D, 0xD6, 0x7E, 0xAB, 0xA0, 0x6F,
    0xB7, 0x13, 0x72, 0x80, 0xA4, 0x47, 0x15, 0xC1, 0x20, 0x99, 0x50, 0xCD,
    0xEC, 0x14, 0x62, 0x09, 0x5B, 0xA4, 0x98, 0xCD, 0xD2, 0x41, 0xB6, 0x36,
    0x4E, 0xFF, 0xE8, 0x2E, 0x32, 0x30, 0x4A, 0x81, 0xA8, 0x42, 0xA3, 0x6C,
    0x9B, 0x33, 0x6E, 0xCA, 0xB2, 0xF5, 0x53, 0x66, 0xE0, 0x27, 0x53, 0x86,
    0x1A, 0x85, 0x1E, 0xA7, 0x39, 0x3F, 0x4A, 0x77, 0x8E, 0xFB, 0x54, 0x66,
    0x66, 0xFB, 0x58, 0x54, 0xC0, 0x5E, 0x39, 0xC7, 0xF5, 0x50, 0x06, 0x0B,
    0xE0, 0x8A, 0xD4, 0xCE, 0xE1, 0x6A, 0x55, 0x1F, 0x8B, 0x17, 0x00, 0xE6,
    0x69, 0xA3, 0x27, 0xE6, 0x08, 0x25, 0x69, 0x3C, 0x12, 0x9D, 0x8D, 0x05,
    0x2C, 0xD6, 0x2E, 0xA2, 0x31, 0xDE, 0xB4, 0x52, 0x50, 0xD6, 0x20, 0x49,
    0xDE, 0x71, 0xA0, 0xF9, 0xAD, 0x20, 0x40, 0x12, 0xF1, 0xDD, 0x25, 0xEB,
    0xD5, 0xE6, 0xB8, 0x36, 0xF4, 0xD6, 0x8F, 0x7F, 0xCA, 0x43, 0xDC, 0xD7,
    0x10, 0x5B, 0xE6, 0x3F, 0x51, 0x8A, 0x85, 0xB3, 0xF3, 0xFF, 0xF6, 0x03,
    0x2D, 0xCB, 0x23, 0x4F, 0x9C, 0xAD, 0x18, 0xE7, 0x93, 0x05, 0x8C, 0xAC,
    0x52, 0x9A, 0xF7, 0x4C, 0xE9, 0x99, 0x7A, 0xBE, 0x6E, 0x7E, 0x4D, 0x0A,
    0xE3, 0xC6, 0x1C, 0xA9, 0x93, 0xFA, 0x3A, 0xA5, 0x91, 0x5D, 0x1C, 0xBD,
    0x66, 0xEB, 0xCC, 0x60, 0xDC, 0x86, 0x74, 0xCA, 0xCF, 0xF8, 0x92, 0x1C,
    0x98, 0x7D, 0x57, 0xFA, 0x61, 0x47, 0x9E, 0xAB, 0x80, 0xB7, 0xE4, 0x48,
    0x80, 0x2A, 0x92, 0xC5, 0x1B, 0x02, 0x03, 0x01, 0x00, 0x01, 0x0A, 0xB3,
    0x02, 0x0A, 0x20, 0xA2, 0x48, 0xC2, 0xE8, 0x54, 0xE6, 0x56, 0xA5, 0x6D,
    0xE8, 0x23, 0x1F, 0x1E, 0xE1, 0x75, 0x6F, 0xDB, 0xE4, 0x07, 0xF9, 0xFE,
    0xD4, 0x65, 0x0D, 0x60, 0xCC, 0x5A, 0xCB, 0x65, 0x11, 0xC7, 0x20, 0x12,
    0x8E, 0x02, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0xB0,
    0x0E, 0x5E, 0x07, 0x3A, 0xDF, 0xA4, 0x5F, 0x68, 0xF7, 0x21, 0xC7, 0x64,
    0xDB, 0xB6, 0x76, 0xEF, 0xEE, 0x8B, 0x93, 0xF8, 0xF6, 0x1B, 0x88, 0xE1,
    0x93, 0xB7, 0x17, 0xF0, 0x15, 0x1E, 0x7E, 0x52, 0x55, 0x77, 0x3C, 0x02,
    0x8D, 0x7B, 0x4A, 0x6C, 0xD3, 0xBD, 0xD6, 0xC1, 0x9C, 0x72, 0xC8, 0xB3,
    0x15, 0xCF, 0x11, 0xC1, 0xF5, 0x46, 0xC4, 0xD5, 0x20, 0x47, 0xFB, 0x30,
    0xF4, 0xE4, 0x61, 0x0C, 0x68, 0xF0, 0x5E, 0xAB, 0x37, 0x8E, 0x9B, 0xE1,
    0xBC, 0x81, 0xC3, 0x70, 0x8A, 0x78, 0xD6, 0x83, 0x34, 0x32, 0x9C, 0x19,
    0x62, 0xEB, 0xE4, 0x9C, 0xED, 0xE3, 0x64, 0x6C, 0x41, 0x1D, 0x9C, 0xD2,
    0x8B, 0x48, 0x4C, 0x23, 0x90, 0x95, 0xB3, 0xE7, 0x52, 0xEA, 0x05, 0x57,
    0xCC, 0x60, 0xB3, 0xBA, 0x14, 0xE4, 0xBA, 0x00, 0x39, 0xE4, 0x46, 0x55,
    0x74, 0xCE, 0x5A, 0x8E, 0x7A, 0x67, 0x23, 0xDA, 0x68, 0x0A, 0xFA, 0xC4,
    0x84, 0x1E, 0xB4, 0xC5, 0xA1, 0xA2, 0x6A, 0x73, 0x1F, 0x6E, 0xC8, 0x2E,
    0x2F, 0x9A, 0x9E, 0xA8, 0xB1, 0x0E, 0xFD, 0x87, 0xA6, 0x8F, 0x4D, 0x3D,
    0x4B, 0x05, 0xD5, 0x35, 0x5A, 0x74, 0x4D, 0xBC, 0x8E, 0x82, 0x44, 0x96,
    0xF4, 0xB5, 0x95, 0x60, 0x4E, 0xA5, 0xDF, 0x27, 0x3D, 0x41, 0x5C, 0x07,
    0xA3, 0xB4, 0x35, 0x5A, 0xB3, 0x9E, 0xF2, 0x05, 0x24, 0xCA, 0xCD, 0x31,
    0x5A, 0x0D, 0x26, 0x4C, 0xD4, 0xD3, 0xFD, 0x50, 0xE1, 0x34, 0xE9, 0x4C,
    0x81, 0x58, 0x30, 0xB2, 0xC7, 0x7A, 0xDD, 0x81, 0x89, 0xA6, 0xD4, 0x3A,
    0x38, 0x84, 0x03, 0xB7, 0x34, 0x9E, 0x77, 0x3F, 0xFF, 0x78, 0x07, 0x5B,
    0x99, 0xC1, 0xB2, 0x1F, 0x35, 0x56, 0x6E, 0x3A, 0x3C, 0x0C, 0x25, 0xE1,
    0x57, 0xF6, 0x8A, 0x7E, 0x49, 0xC0, 0xCC, 0x83, 0x11, 0x35, 0xE7, 0x91,
    0x6D, 0x2E, 0x65, 0x02, 0x03, 0x01, 0x00, 0x01
];

var INVALID_AUTHORITY_KEYS = [
    0x10, 0xB3, 0x02, 0x0A, 0x20, 0x52, 0x9D, 0x9C, 0xD6, 0x7F, 0xE5, 0xEB,
    0x69, 0x8E, 0x70, 0xDD, 0x26, 0xD7, 0xD8, 0xF1, 0x26, 0x59, 0xF1, 0xE6,
    0xE5, 0x23, 0x48, 0xBF, 0x6A, 0x5C, 0xF7, 0x16, 0xE1, 0x3F, 0x41, 0x0E,
    0x73, 0x12, 0x8E, 0x02, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01,
    0x00, 0xBC, 0x22, 0x80, 0xBD, 0x80, 0xF6, 0x3A, 0x21, 0x00, 0x3B, 0xAE,
    0x76, 0x5E, 0x35, 0x7F, 0x3D, 0xC3, 0x64, 0x5C, 0x55, 0x94, 0x86, 0x34,
    0x2F, 0x05, 0x87, 0x28, 0xCD, 0xF7, 0x69, 0x8C, 0x17, 0xB3, 0x50, 0xA7,
    0xB8, 0x82, 0xFA, 0xDF, 0xC7, 0x43, 0x2D, 0xD6, 0x7E, 0xAB, 0xA0, 0x6F,
    0xB7, 0x13, 0x72, 0x80, 0xA4, 0x47, 0x15, 0xC1, 0x20, 0x99, 0x50, 0xCD,
    0xEC, 0x14, 0x62, 0x09, 0x5B, 0xA4, 0x98, 0xCD, 0xD2, 0x41, 0xB6, 0x36,
    0x4E, 0xFF, 0xE8, 0x2E, 0x32, 0x30, 0x4A, 0x81, 0xA8, 0x42, 0xA3, 0x6C,
    0x9B, 0x33, 0x6E, 0xCA, 0xB2, 0xF5, 0x53, 0x66, 0xE0, 0x27, 0x53, 0x86,
    0x1A, 0x85, 0x1E, 0xA7, 0x39, 0x3F, 0x4A, 0x77, 0x8E, 0xFB, 0x54, 0x66,
    0x66, 0xFB, 0x58, 0x54, 0xC0, 0x5E, 0x39, 0xC7, 0xF5, 0x50, 0x06, 0x0B,
    0xE0, 0x8A, 0xD4, 0xCE, 0xE1, 0x6A, 0x55, 0x1F, 0x8B, 0x17, 0x00, 0xE6,
    0x69, 0xA3, 0x27, 0xE6, 0x08, 0x25, 0x69, 0x3C, 0x12, 0x9D, 0x8D, 0x05,
    0x2C, 0xD6, 0x2E, 0xA2, 0x31, 0xDE, 0xB4, 0x52, 0x50, 0xD6, 0x20, 0x49,
    0xDE, 0x71, 0xA0, 0xF9, 0xAD, 0x20, 0x40, 0x12, 0xF1, 0xDD, 0x25, 0xEB,
    0xD5, 0xE6, 0xB8, 0x36, 0xF4, 0xD6, 0x8F, 0x7F, 0xCA, 0x43, 0xDC, 0xD7,
    0x10, 0x5B, 0xE6, 0x3F, 0x51, 0x8A, 0x85, 0xB3, 0xF3, 0xFF, 0xF6, 0x03,
    0x2D, 0xCB, 0x23, 0x4F, 0x9C, 0xAD, 0x18, 0xE7, 0x93, 0x05, 0x8C, 0xAC,
    0x52, 0x9A, 0xF7, 0x4C, 0xE9, 0x99, 0x7A, 0xBE, 0x6E, 0x7E, 0x4D, 0x0A,
    0xE3, 0xC6, 0x1C, 0xA9, 0x93, 0xFA, 0x3A, 0xA5, 0x91, 0x5D, 0x1C, 0xBD,
    0x66, 0xEB, 0xCC, 0x60, 0xDC, 0x86, 0x74, 0xCA, 0xCF, 0xF8, 0x92, 0x1C,
    0x98, 0x7D, 0x57, 0xFA, 0x61, 0x47, 0x9E, 0xAB, 0x80, 0xB7, 0xE4, 0x48,
    0x80, 0x2A, 0x92, 0xC5, 0x1B, 0x02, 0x03, 0x01, 0x00, 0x01, 0x0A, 0xB3,
    0x02, 0x0A, 0x20, 0xA2, 0x48, 0xC2, 0xE8, 0x54, 0xE6, 0x56, 0xA5, 0x6D,
    0xE8, 0x23, 0x1F, 0x1E, 0xE1, 0x75, 0x6F, 0xDB, 0xE4, 0x07, 0xF9, 0xFE,
    0xD4, 0x65, 0x0D, 0x60, 0xCC, 0x5A, 0xCB, 0x65, 0x11, 0xC7, 0x20, 0x12,
    0x8E, 0x02, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0xB0,
    0x0E, 0x5E, 0x07, 0x3A, 0xDF, 0xA4, 0x5F, 0x68, 0xF7, 0x21, 0xC7, 0x64,
    0xDB, 0xB6, 0x76, 0xEF, 0xEE, 0x8B, 0x93, 0xF8, 0xF6, 0x1B, 0x88, 0xE1,
    0x93, 0xB7, 0x17, 0xF0, 0x15, 0x1E, 0x7E, 0x52, 0x55, 0x77, 0x3C, 0x02,
    0x8D, 0x7B, 0x4A, 0x6C, 0xD3, 0xBD, 0xD6, 0xC1, 0x9C, 0x72, 0xC8, 0xB3,
    0x15, 0xCF, 0x11, 0xC1, 0xF5, 0x46, 0xC4, 0xD5, 0x20, 0x47, 0xFB, 0x30,
    0xF4, 0xE4, 0x61, 0x0C, 0x68, 0xF0, 0x5E, 0xAB, 0x37, 0x8E, 0x9B, 0xE1,
    0xBC, 0x81, 0xC3, 0x70, 0x8A, 0x78, 0xD6, 0x83, 0x34, 0x32, 0x9C, 0x19,
    0x62, 0xEB, 0xE4, 0x9C, 0xED, 0xE3, 0x64, 0x6C, 0x41, 0x1D, 0x9C, 0xD2,
    0x8B, 0x48, 0x4C, 0x23, 0x90, 0x95, 0xB3, 0xE7, 0x52, 0xEA, 0x05, 0x57,
    0xCC, 0x60, 0xB3, 0xBA, 0x14, 0xE4, 0xBA, 0x00, 0x39, 0xE4, 0x46, 0x55,
    0x74, 0xCE, 0x5A, 0x8E, 0x7A, 0x67, 0x23, 0xDA, 0x68, 0x0A, 0xFA, 0xC4,
    0x84, 0x1E, 0xB4, 0xC5, 0xA1, 0xA2, 0x6A, 0x73, 0x1F, 0x6E, 0xC8, 0x2E,
    0x2F, 0x9A, 0x9E, 0xA8, 0xB1, 0x0E, 0xFD, 0x87, 0xA6, 0x8F, 0x4D, 0x3D,
    0x4B, 0x05, 0xD5, 0x35, 0x5A, 0x74, 0x4D, 0xBC, 0x8E, 0x82, 0x44, 0x96,
    0xF4, 0xB5, 0x95, 0x60, 0x4E, 0xA5, 0xDF, 0x27, 0x3D, 0x41, 0x5C, 0x07,
    0xA3, 0xB4, 0x35, 0x5A, 0xB3, 0x9E, 0xF2, 0x05, 0x24, 0xCA, 0xCD, 0x31,
    0x5A, 0x0D, 0x26, 0x4C, 0xD4, 0xD3, 0xFD, 0x50, 0xE1, 0x34, 0xE9, 0x4C,
    0x81, 0x58, 0x30, 0xB2, 0xC7, 0x7A, 0xDD, 0x81, 0x89, 0xA6, 0xD4, 0x3A,
    0x38, 0x84, 0x03, 0xB7, 0x34, 0x9E, 0x77, 0x3F, 0xFF, 0x78, 0x07, 0x5B,
    0x99, 0xC1, 0xB2, 0x1F, 0x35, 0x56, 0x6E, 0x3A, 0x3C, 0x0C, 0x25, 0xE1,
    0x57, 0xF6, 0x8A, 0x7E, 0x49, 0xC0, 0xCC, 0x83, 0x11, 0x35, 0xE7, 0x91,
    0x6D, 0x2E, 0x65, 0x02, 0x03, 0x01, 0x00, 0x01
];

var generateBase64Data = function(s) {
  return window.btoa(String.fromCharCode.apply(null, s));
};

var onInvalidAuthorityKeys = function() {
  chrome.test.assertLastError('Unable to set authority keys.');
  chrome.test.succeed();
};

var onValidAuthorityKeys = function() {
  chrome.test.assertNoLastError();
  chrome.test.succeed();
};

chrome.test.runTests([
  function testValid() {
    chrome.cast.channel.setAuthorityKeys(
        generateBase64Data(VALID_AUTHORITY_KEYS),
        generateBase64Data(VALID_SIGNATURE), onValidAuthorityKeys);
  },
  function testInvalid() {
    chrome.cast.channel.setAuthorityKeys(
        generateBase64Data(INVALID_AUTHORITY_KEYS),
        generateBase64Data(VALID_SIGNATURE), onInvalidAuthorityKeys);
  }
]);