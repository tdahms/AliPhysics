/**************************************************************************
 * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
/*
 * Base class for anaysis components. Inheriting classes have to implement the
 * functions CreateHistos and Process.
 *
 *   Author: Markus Fasel
 */
#include "AliEMCalTriggerTracksAnalysisComponent.h"

ClassImp(EMCalTriggerPtAnalysis::AliEMCalTriggerTracksAnalysisComponent)

namespace EMCalTriggerPtAnalysis {

//______________________________________________________________________________
AliEMCalTriggerTracksAnalysisComponent::AliEMCalTriggerTracksAnalysisComponent() :
  TNamed(),
  fHistos(NULL),
  fBinning(NULL),
  fKineCuts(NULL)
{
  /*
   * Dummy (I/O) constructor
   */
}

//______________________________________________________________________________
AliEMCalTriggerTracksAnalysisComponent::~AliEMCalTriggerTracksAnalysisComponent() {
  /*
   * Release histogram container
   */
  if(fHistos) delete fHistos;
}

//______________________________________________________________________________
AliEMCalTriggerTracksAnalysisComponent::AliEMCalTriggerTracksAnalysisComponent(const char* name) :
  TNamed(name,""),
  fHistos(NULL),
  fBinning(NULL),
  fKineCuts(NULL)
{
  /*
   * Main constructor, to be called by the user
   *
   * @param name: component name
   */
}

//______________________________________________________________________________
void AliEMCalTriggerTracksAnalysisComponent::CreateHistos() {
  /*
   * Create Container for histograms. Inheriting classes overwrite this method, in which they call
   * this and add the histograms of their choise.
   */
  fHistos = new AliEMCalHistoContainer(Form("Histos%s", GetName()));
  fHistos->ReleaseOwner();
}

} /* namespace EMCalTriggerPtAnalysis */

