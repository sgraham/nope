/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// GMM tables for inactive segments. Generated by MakeGmmTables.m.

#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_AGC_NOISE_GMM_TABLES_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_AGC_NOISE_GMM_TABLES_H_

static const int kNoiseGmmNumMixtures = 12;
static const int kNoiseGmmDim = 3;

static const double kNoiseGmmCovarInverse[kNoiseGmmNumMixtures]
                                          [kNoiseGmmDim][kNoiseGmmDim] = {
  {{ 7.36219567592941e+00,  4.83060785179861e-03,  1.23335151497610e-02},
   { 4.83060785179861e-03,  1.65289507047817e-04, -2.41490588169997e-04},
   { 1.23335151497610e-02, -2.41490588169997e-04,  6.59472060689382e-03}},
  {{ 8.70265239309140e+00, -5.30636201431086e-04,  5.44014966585347e-03},
   {-5.30636201431086e-04,  3.11095453521008e-04, -1.86287206836035e-04},
   { 5.44014966585347e-03, -1.86287206836035e-04,  6.29493388790744e-04}},
  {{ 4.53467851955055e+00, -3.92977536695197e-03, -2.46521420693317e-03},
   {-3.92977536695197e-03,  4.94650752632750e-05, -1.08587438501826e-05},
   {-2.46521420693317e-03, -1.08587438501826e-05,  9.28793975422261e-05}},
  {{ 9.26817997114275e-01, -4.03976069276753e-04, -3.56441427392165e-03},
   {-4.03976069276753e-04,  2.51976251631430e-06,  1.46914206734572e-07},
   {-3.56441427392165e-03,  1.46914206734572e-07,  8.19914567685373e-05}},
  {{ 7.61715986787441e+00, -1.54889041216888e-04,  2.41756280071656e-02},
   {-1.54889041216888e-04,  3.50282550461672e-07, -6.27251196972490e-06},
   { 2.41756280071656e-02, -6.27251196972490e-06,  1.45061847649872e-02}},
  {{ 8.31193642663158e+00, -3.84070508164323e-04, -3.09750630821876e-02},
   {-3.84070508164323e-04,  3.80433432277336e-07, -1.14321142836636e-06},
   {-3.09750630821876e-02, -1.14321142836636e-06,  8.35091486289997e-04}},
  {{ 9.67283151270894e-01,  5.82465812445039e-05, -3.18350798617053e-03},
   { 5.82465812445039e-05,  2.23762672000318e-07, -7.74196587408623e-07},
   {-3.18350798617053e-03, -7.74196587408623e-07,  3.85120938338325e-04}},
  {{ 8.28066236985388e+00,  5.87634508319763e-05,  6.99303090891743e-03},
   { 5.87634508319763e-05,  2.93746018618058e-07,  3.40843332882272e-07},
   { 6.99303090891743e-03,  3.40843332882272e-07,  1.99379171190344e-04}},
  {{ 6.07488998675646e+00, -1.11494526618473e-02,  5.10013111123381e-03},
   {-1.11494526618473e-02,  6.99238879921751e-04,  5.36718550370870e-05},
   { 5.10013111123381e-03,  5.36718550370870e-05,  5.26909853276753e-04}},
  {{ 6.90492021419175e+00,  4.20639355257863e-04, -2.38612752336481e-03},
   { 4.20639355257863e-04,  3.31246767338153e-06, -2.42052288150859e-08},
   {-2.38612752336481e-03, -2.42052288150859e-08,  4.46608368363412e-04}},
  {{ 1.31069150869715e+01, -1.73718583865670e-04, -1.97591814508578e-02},
   {-1.73718583865670e-04,  2.80451716300124e-07,  9.96570755379865e-07},
   {-1.97591814508578e-02,  9.96570755379865e-07,  2.41361900868847e-03}},
  {{ 4.69566344239814e+00, -2.61077567563690e-04,  5.26359000761433e-03},
   {-2.61077567563690e-04,  1.82420859823767e-06, -7.83645887541601e-07},
   { 5.26359000761433e-03, -7.83645887541601e-07,  1.33586288288802e-02}}};

static const double kNoiseGmmMean[kNoiseGmmNumMixtures][kNoiseGmmDim] = {
  {-2.01386094766163e+00,  1.69702162045397e+02,  7.41715804872181e+01},
  {-1.94684591777290e+00,  1.42398396732668e+02,  1.64186321157831e+02},
  {-2.29319297562437e+00,  3.86415425589868e+02,  2.13452215267125e+02},
  {-3.25487177070268e+00,  1.08668712553616e+03,  2.33119949467419e+02},
  {-2.13159632447467e+00,  4.83821702557717e+03,  6.86786166673740e+01},
  {-2.26171410780526e+00,  4.79420193982422e+03,  1.53222513286450e+02},
  {-3.32166740703185e+00,  4.35161135834358e+03,  1.33206448431316e+02},
  {-2.19290322814343e+00,  3.98325506609408e+03,  2.13249167359934e+02},
  {-2.02898459255404e+00,  7.37039893155007e+03,  1.12518527491926e+02},
  {-2.26150236399500e+00,  1.54896745196145e+03,  1.49717357868579e+02},
  {-2.00417668301790e+00,  3.82434760310304e+03,  1.07438913004312e+02},
  {-2.30193040814533e+00,  1.43953696546439e+03,  7.04085275122649e+01}};

static const double kNoiseGmmWeights[kNoiseGmmNumMixtures] = {
  -1.09422832086193e+01, -1.10847897513425e+01, -1.36767587732187e+01,
  -1.79789356118641e+01, -1.42830169160894e+01, -1.56500228061379e+01,
  -1.83124990950113e+01, -1.69979436177477e+01, -1.12329424387828e+01,
  -1.41311785780639e+01, -1.47171861448585e+01, -1.35963362781839e+01};
#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_AGC_NOISE_GMM_TABLES_H_