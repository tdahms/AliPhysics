#ifndef ALIANALYSISTASKNUCLEIKINE_H
#define ALIANALYSISTASKNUCLEIKINE_H

#include "AliAnalysisTaskSE.h"

#include <TMath.h>

#include "Math/GenVector/PxPyPzE4D.h"
#include "Math/GenVector/LorentzVector.h"
using ROOT::Math::PxPyPzE4D;
using ROOT::Math::LorentzVector;

#include <string>
using std::string;

#include <vector>
using std::vector;
#include <utility>
using std::pair;
#include <TF1.h>

class TH1D;
class TH2D;
class TList;

class AliAnalysisTaskNucleiKine: public AliAnalysisTaskSE {
  public:
    AliAnalysisTaskNucleiKine(const string name = "AliAnalysisTaskNucleiKine");
    virtual ~AliAnalysisTaskNucleiKine();

    virtual void UserCreateOutputObjects();
    virtual void UserExec(Option_t *option);
    virtual void Terminate(Option_t *opt) {}

    template<typename F> static F GetPcm(const LorentzVector<PxPyPzE4D<F> > &a, const LorentzVector<PxPyPzE4D<F> > &b);

    /// Coalescence parameters
    TF1   fPotentialShape;
    float fSpinProb;

    /// Spatial distribution
    TF1   fSpatialDistribution;
    bool  fEnableDisplace;

    // POIs
    vector<int> fPdgCodes;
    enum Species {
      kPiPlus, kPiMinus, kKplus, kKminus, kProton, kAntiProton, kNeutron, kAntiNeutron, kDeuteron, kAntiDeuteron
    };

    struct Particle {
      ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > pos;
      ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > mom;
    };

  protected:
    AliAnalysisTaskNucleiKine(const AliAnalysisTaskNucleiKine& other);
    AliAnalysisTaskNucleiKine& operator=(const AliAnalysisTaskNucleiKine& other);

    void FirstPartner(int iS, vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > > &d);

    TList* fOutputList;    //! output list for histograms

    TH1D*  fEventCounter;  //!
    TH2D*  fPtSpectra;     //!
    TH2D*  fCosine;        //!
    TH1D*  fPsi2;          //!

    vector<bool> fMask;
    vector<pair<int,Particle> > fNeutrons[2];
    vector<pair<int,Particle> > fProtons[2];

    ClassDef(AliAnalysisTaskNucleiKine, 2)
};

template<typename F> F AliAnalysisTaskNucleiKine::GetPcm(const ROOT::Math::LorentzVector<PxPyPzE4D<F> > &a, const ROOT::Math::LorentzVector<PxPyPzE4D<F> > &b) {
  LorentzVector<PxPyPzE4D<F> > c = a + b;
  const F s = c.mass2();
  const F dm2 = (a.mass() - b.mass()) * (a.mass() - b.mass());
  return TMath::Abs(s - dm2) / (2.f * c.mass());
}

#endif