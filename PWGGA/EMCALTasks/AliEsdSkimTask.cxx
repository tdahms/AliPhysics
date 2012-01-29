// $Id$
//
// Task to skim ESD files.
//
//

#include "AliEsdSkimTask.h"
#include <TClonesArray.h>
#include <TFile.h>
#include <TTree.h>
#include "AliAnalysisManager.h"
#include "AliCentrality.h"
#include "AliESDEvent.h"
#include "AliESDFMD.h"
#include "AliESDtrackCuts.h"
#include "AliEsdTrackExt.h"
#include "AliEventplane.h"
#include "AliMultiplicity.h"
#include "AliVEventHandler.h"
#include "AliPicoTrack.h"

//_________________________________________________________________________________________________
AliEsdSkimTask::AliEsdSkimTask(const char *opt) :
  AliAnalysisTaskSE(opt), fEvent(0), fTree(0), fCuts(0),
  fDoZDC(1), fDoV0(1), fDoT0(1), fDoTPCv(1), fDoSPDv(1), fDoPriv(1),
  fDoEmCs(1), fDoPCs(0), fDoEmT(1), fDoPT(0), fDoTracks(0), fDoFmd(1),
  fDoMult(1), fDoTof(0), fDoPileup(1), fDoClus(0), fEmcNames(""), 
  fDoMiniTracks(0), fTracks("Tracks"), fPhosClusOnly(0), fDoSaveBytes(0),
  fDoCent(1), fDoRP(1), fRemoveCP(0), fResetCov(1), fDoPicoTracks(0)
{
  // Constructor.

  if (!opt)
    return;

  fBranchNames = "ESD:AliESDHeader.,AliESDRun.";

  DefineOutput(1, TTree::Class());
}

//_________________________________________________________________________________________________
void AliEsdSkimTask::UserExec(Option_t */*opt*/) 
{
  // Process event.

  AliESDEvent *esdin = dynamic_cast<AliESDEvent*>(InputEvent());
  if (!esdin)
    return;

  AliAnalysisManager *am = AliAnalysisManager::GetAnalysisManager();

  fEvent->ResetStdContent();

  TList* objsin  = esdin->GetList();
  TList* objsout = fEvent->GetList();

  AliESDHeader *header = dynamic_cast<AliESDHeader*>(objsout->FindObject("AliESDHeader"));
  if (header) {
    am->LoadBranch("AliESDHeader.");
    if (!fDoSaveBytes) {
      *header = *esdin->GetHeader();
    } else {
      AliESDHeader *hin = esdin->GetHeader();
      header->SetTriggerMask(hin->GetTriggerMask());
      header->SetOrbitNumber(hin->GetOrbitNumber());
      header->SetTimeStamp(hin->GetTimeStamp());
      header->SetEventType(hin->GetEventType());
      header->SetEventSpecie(hin->GetEventSpecie());
      header->SetEventNumberInFile(hin->GetEventNumberInFile());
      header->SetBunchCrossNumber(hin->GetBunchCrossNumber());
      header->SetPeriodNumber(hin->GetPeriodNumber());
      header->SetTriggerCluster(hin->GetTriggerCluster());
      header->SetL0TriggerInputs(hin->GetL0TriggerInputs());
      header->SetL1TriggerInputs(hin->GetL1TriggerInputs());
      header->SetL2TriggerInputs(hin->GetL2TriggerInputs());
      for (Int_t i=0;i<24;++i) {
        const char *name = hin->GetTriggerInputName(i,0);
        if (name)
          header->SetActiveTriggerInputs(name,i);
      }
      for (Int_t i=0;i<24;++i) {
        const char *name = hin->GetTriggerInputName(i,1);
        if (name)
          header->SetActiveTriggerInputs(name,i+24);
      }
      for (Int_t i=0;i<12;++i) {
        const char *name = hin->GetTriggerInputName(i,2);
        if (name)
          header->SetActiveTriggerInputs(name,i+48);
      }
    }
    if (am->GetInputEventHandler()) {
      TTree *tree = am->GetInputEventHandler()->GetTree();
      if (tree) {
        TFile *cfile = tree->GetCurrentFile();
        if (cfile) {
          TString cname(cfile->GetName());
          header->SetTitle(cname);
        }
      }
    }
  }
  AliESDRun *run = dynamic_cast<AliESDRun*>(objsout->FindObject("AliESDRun"));
  if (run) {
    am->LoadBranch("AliESDRun.");
    *run = *esdin->GetESDRun();
  }
  AliCentrality *cent = dynamic_cast<AliCentrality*>(objsout->FindObject("Cent"));
  if (cent) {
    cent->Reset();
    AliCentrality *centin = esdin->GetCentrality();
    cent->SetQuality(centin->GetQuality());
    cent->SetCentralityV0M(centin->GetCentralityPercentileUnchecked("V0M"));
    cent->SetCentralityFMD(centin->GetCentralityPercentileUnchecked("FMD"));
    cent->SetCentralityTRK(centin->GetCentralityPercentileUnchecked("TRK"));
    cent->SetCentralityTKL(centin->GetCentralityPercentileUnchecked("TKL"));
    cent->SetCentralityCL0(centin->GetCentralityPercentileUnchecked("CL0"));
    cent->SetCentralityCL1(centin->GetCentralityPercentileUnchecked("CL1"));
    cent->SetCentralityV0MvsFMD(centin->GetCentralityPercentileUnchecked("V0MvsFMD"));
    cent->SetCentralityTKLvsV0M(centin->GetCentralityPercentileUnchecked("TKLvsV0M"));
    cent->SetCentralityZEMvsZDC(centin->GetCentralityPercentileUnchecked("ZEMvsZDC"));
  }
  AliEventplane *ep = dynamic_cast<AliEventplane*>(objsout->FindObject("EP"));
  if (ep) {
    ep->Reset();
    AliEventplane *epin = esdin->GetEventplane();
    if (!fDoSaveBytes) {
      *ep = *epin;
    } else {
      if (epin->GetQVector()) {
        ep->SetQVector(new TVector2(*epin->GetQVector()));
        ep->SetEventplaneQ(epin->GetEventplane("Q"));
        ep->SetQsub(new TVector2(*epin->GetQsub1()),new TVector2(*epin->GetQsub2()));
        ep->SetQsubRes(epin->GetQsubRes());
      }
    }
  }
  AliESDZDC *zdc = dynamic_cast<AliESDZDC*>(objsout->FindObject("AliESDZDC"));
  if (zdc) {
    am->LoadBranch("AliESDZDC.");
    *zdc = *esdin->GetESDZDC();
  }
  AliESDVZERO *v0 = dynamic_cast<AliESDVZERO*>(objsout->FindObject("AliESDVZERO"));
  if (v0) {
    am->LoadBranch("AliESDVZERO.");
    *v0 = *esdin->GetVZEROData();
  }
  AliESDTZERO *t0 = dynamic_cast<AliESDTZERO*>(objsout->FindObject("AliESDTZERO"));
  if (t0) {
    am->LoadBranch("AliESDTZERO.");
    *t0 = *esdin->GetESDTZERO();
  }
  AliESDVertex *tpcv = dynamic_cast<AliESDVertex*>(objsout->FindObject("TPCVertex"));
  if (tpcv) {
    am->LoadBranch("TPCVertex.");
    *tpcv = *esdin->GetPrimaryVertexTPC();
  }
  AliESDVertex *spdv = dynamic_cast<AliESDVertex*>(objsout->FindObject("SPDVertex"));
  if (spdv) {
    am->LoadBranch("SPDVertex.");
    *spdv = *esdin->GetPrimaryVertexSPD();
  }
  AliESDVertex *priv = dynamic_cast<AliESDVertex*>(objsout->FindObject("PrimaryVertex"));
  if (priv) {
    am->LoadBranch("PrimaryVertex.");
    *priv = *esdin->GetPrimaryVertexTracks();
  }
#if 1
  AliESDCaloCells *ecells = dynamic_cast<AliESDCaloCells*>(objsout->FindObject("EMCALCells"));
  if (ecells) {
    am->LoadBranch("EMCALCells.");
    *ecells = *esdin->GetEMCALCells();
  }
#else
  TClonesArray *ecells = dynamic_cast<TClonesArray*>(objsout->FindObject("EMCALCells"));
  if (ecells) {
    am->LoadBranch("EMCALCells.");
    AliESDCaloCells *ein = esdin->GetEMCALCells();
    ecells->Clear();
    for(Int_t i=0;i<ein->GetNumberOfCells();++i) {
      Short_t  cellNumber = 0;
      Double_t amplitude  = 0; 
      Double_t time       = 0;
      ein->GetCell(i, cellNumber, amplitude, time);
      new ((*ecells)[i]) AliCaloCell(cellNumber, amplitude, time);
    }
  }
#endif

  AliESDCaloCells *pcells = dynamic_cast<AliESDCaloCells*>(objsout->FindObject("PHOSCells"));
  if (pcells) {
    am->LoadBranch("PHOSCells.");
    *pcells = *esdin->GetPHOSCells();
  }
  AliESDCaloTrigger *etrig = dynamic_cast<AliESDCaloTrigger*>(objsout->FindObject("EMCALTrigger"));
  if (etrig) {
    am->LoadBranch("EMCALTrigger.");
    *etrig = *esdin->GetCaloTrigger("EMCAL");
    etrig->Print("");
  }
  AliESDCaloTrigger *ptrig = dynamic_cast<AliESDCaloTrigger*>(objsout->FindObject("PHOSTrigger"));
  if (ptrig) {
    am->LoadBranch("PHOSTrigger.");
    *ptrig = *esdin->GetCaloTrigger("PHOS");
  }
  AliESDFMD *fmd = dynamic_cast<AliESDFMD*>(objsout->FindObject("AliESDFMD"));
  if (fmd) {
    am->LoadBranch("AliESDFMD.");
    if (!fDoSaveBytes) {
      *fmd = *esdin->GetFMDData();
    }
  }
  AliMultiplicity *mult = dynamic_cast<AliMultiplicity*>(objsout->FindObject("AliMultiplicity"));
  if (mult) {
    am->LoadBranch("AliMultiplicity.");
    if (!fDoSaveBytes) {
      *mult = *esdin->GetMultiplicity();
    } else {
      const AliMultiplicity *multin = esdin->GetMultiplicity();;
      mult->SetFiredChips(0, multin->GetNumberOfFiredChips(0));
      mult->SetFiredChips(1, multin->GetNumberOfFiredChips(1));
      for (Int_t i=0; i<6; ++i) 
        mult->SetITSClusters(i,mult->GetNumberOfITSClusters(i));
    }
  }
  AliTOFHeader *tofh = dynamic_cast<AliTOFHeader*>(objsout->FindObject("AliTOFHeader"));
  if (tofh) {
    am->LoadBranch("AliTOFHeader.");
    *tofh = *esdin->GetTOFHeader();
  }
  TClonesArray *spup = dynamic_cast<TClonesArray*>(objsout->FindObject("SPDPileupVertices"));
  if (spup) {
    am->LoadBranch("SPDPileupVertices");
    Int_t N = esdin->GetNumberOfPileupVerticesSPD();
    for (Int_t i=0; i<N; ++i) {
      const AliESDVertex *vtx = esdin->GetPileupVertexSPD(i);
      if (vtx)
        fEvent->AddPileupVertexSPD(vtx);
    }
  }
  TClonesArray *tpup = dynamic_cast<TClonesArray*>(objsout->FindObject("TrkPileupVertices"));
  if (tpup) {
    am->LoadBranch("TrkPileupVertices");
    Int_t N = esdin->GetNumberOfPileupVerticesTracks();
    for (Int_t i=0; i<N; ++i) {
      const AliESDVertex *vtx = esdin->GetPileupVertexTracks(i);
      if (vtx)
        fEvent->AddPileupVertexTracks(vtx);
    }
  }
  TClonesArray *clus = dynamic_cast<TClonesArray*>(objsout->FindObject("CaloClusters"));
  if (clus) {
    am->LoadBranch("CaloClusters");
    Int_t N = esdin->GetNumberOfCaloClusters();
    for (Int_t i=0; i<N; ++i) {
      AliESDCaloCluster *c = esdin->GetCaloCluster(i);
      if (fPhosClusOnly && c->IsEMCAL())
        continue;
      if (c)
        fEvent->AddCaloCluster(c);
    }
  }
  TObjArray *namearr = fEmcNames.Tokenize(";");
  if (namearr) {
    for (Int_t i=0; i<namearr->GetEntries(); ++i) {
      TString cname(namearr->At(i)->GetName());
      if (cname.Length()<=0)
        continue;
      TClonesArray *arrin  = dynamic_cast<TClonesArray*>(objsin->FindObject(cname));
      if (!arrin) {
        AliFatal(Form("Can not find input clusters with name %s", cname.Data()));
        return;
      }
      TClonesArray *arrout = dynamic_cast<TClonesArray*>(objsout->FindObject(cname));
      if (!arrout) {
        AliFatal(Form("Can not find output clusters with name %s", cname.Data()));
        return;
      }
      arrout->Delete();
      Double_t emin=0.1;
      if (cname.Contains("FEE"))
        emin = 1;
      const Int_t N = arrin->GetEntries();
      for (Int_t iC=0, nC=0; iC<N; ++iC) {
        AliESDCaloCluster *c = dynamic_cast<AliESDCaloCluster*>(arrin->At(iC));
        if (!c)
          continue;
        if (c->E()<emin)
          continue;
        AliESDCaloCluster *newCluster = new ((*arrout)[nC]) AliESDCaloCluster(*c);
        newCluster->SetID(nC);
        ++nC;
      }
    }
    delete namearr;
  }
  if (fDoTracks) {
    am->LoadBranch("Tracks");
    TClonesArray *tracksin = dynamic_cast<TClonesArray*>(objsin->FindObject(fTracks));
    if (!tracksin) {
      AliFatal(Form("Can not find tracks with name %s", fTracks.Data()));
      return;
    }
    TClonesArray *tracksout = dynamic_cast<TClonesArray*>(objsout->FindObject("Tracks"));
    if (!tracksout) {
      AliFatal(Form("Can not find tracks with name %s", "Tracks"));
      return;
    }
    const Int_t Ntracks = tracksin->GetEntries();
    Int_t nacc = 0;
    for (Int_t iTracks = 0; iTracks < Ntracks; ++iTracks) {
      if (iTracks>1500) break;
      AliESDtrack *track = dynamic_cast<AliESDtrack*>(tracksin->At(iTracks));
      if (!track)
        continue;
      if (fCuts) {
        if (!fCuts->IsSelected(track))
          continue;
      }
      if (fDoPicoTracks) {
        AliEsdTrackExt newtrack(*track);
        Double_t etaemc = 0;
        Double_t phiemc = 0;
        if (newtrack.IsEMCAL()) {
          etaemc = newtrack.GetEmcEta();
          phiemc = newtrack.GetEmcPhi();
        }
        new ((*tracksout)[nacc]) AliPicoTrack(newtrack.Pt(), newtrack.Eta(), newtrack.Phi(), 
                                              newtrack.Charge(), newtrack.GetLabel(), 
                                              etaemc, phiemc, newtrack.IsEMCAL());
        ++nacc;
      } else {
        AliEsdTrackExt *newtrack = new ((*tracksout)[nacc]) AliEsdTrackExt(*track);
        if (fDoMiniTracks) {
          newtrack->MakeMiniTrack(0,fRemoveCP);
          newtrack->ResetCovariance(fResetCov);
        } else {
          newtrack->DeleteParams();
        }
        newtrack->SetID(nacc);
        ++nacc;
      }
    }
    if (fCuts) 
      AliInfo(Form("Selected %d out of %d \n", nacc, Ntracks));
  }
  fTree->Fill();
}

//_________________________________________________________________________________________________
void AliEsdSkimTask::UserCreateOutputObjects() 
{
  // Create output objects.

  TFile *file = OpenFile(1);
  fTree = new TTree("esdTree", "Tree with skimmed ESD objects");
  file->SetCompressionLevel(2);
  fTree->SetDirectory(file);
  fTree->SetAutoFlush(-10*1024*1024);

  fEvent = new AliESDEvent;
  fEvent->AddObject(new AliESDHeader());
  fEvent->AddObject(new AliESDRun());
  if (fDoCent) {
    AliCentrality *cent = new AliCentrality;
    cent->SetName("Cent");
    fEvent->AddObject(cent);
  }
  if (fDoRP) {
    AliEventplane *ep = new AliEventplane;
    ep->SetName("EP");
    fEvent->AddObject(ep);
  }
  if (fDoZDC) 
    fEvent->AddObject(new AliESDZDC());
  if (fDoV0)
    fEvent->AddObject(new AliESDVZERO());
  if (fDoT0)
    fEvent->AddObject(new AliESDTZERO());
  if (fDoTPCv) {
    AliESDVertex *tpcv = new AliESDVertex();
    tpcv->SetName("TPCVertex");
    fEvent->AddObject(tpcv);
  }
  if (fDoSPDv) {
    AliESDVertex *spdv = new AliESDVertex();
    spdv->SetName("SPDVertex");
    fEvent->AddObject(spdv);
  }
  if (fDoPriv) {
    AliESDVertex *priv = new AliESDVertex();
    priv->SetName("PrimaryVertex");
    fEvent->AddObject(priv);
  }
  if (fDoEmCs) {
    fEvent->AddObject(new AliESDCaloCells("EMCALCells","EMCALCells"));
    //TClonesArray *arr = new TClonesArray("AliCaloCell",0);
    //arr->SetName("EMCALCells");
    //fEvent->AddObject(arr);
  }
  if (fDoPCs) {
    fEvent->AddObject(new AliESDCaloCells("PHOSCells","PHOSCells"));
  }
  if (fDoEmT) {
    AliESDCaloTrigger *etrig = new AliESDCaloTrigger;
    etrig->SetName("EMCALTrigger");
    fEvent->AddObject(etrig);
  }
  if (fDoPT) {
    AliESDCaloTrigger *ptrig = new AliESDCaloTrigger;
    ptrig->SetName("PHOSTrigger");
    fEvent->AddObject(ptrig);
  }
  if (fDoFmd) {
    AliESDFMD *fmd = new AliESDFMD;
    fEvent->AddObject(fmd);
  }
  if (fDoMult) {
    fEvent->AddObject(new AliMultiplicity());
  }
  if (fDoPileup) {
    TClonesArray *arr1 = new TClonesArray("AliESDVertex",0);
    arr1->SetName("SPDPileupVertices");
    fEvent->AddObject(arr1);
    TClonesArray *arr2 = new TClonesArray("AliESDVertex",0);
    arr2->SetName("TrkPileupVertices");
    fEvent->AddObject(arr2);
  }
  if (fDoTof) { 
    fEvent->AddObject(new AliTOFHeader());
  }
  if (fDoClus) {
    TClonesArray *arr = new TClonesArray("AliESDCaloCluster",0);
    arr->SetName("CaloClusters");
    fEvent->AddObject(arr);
  }
  TObjArray *namearr = fEmcNames.Tokenize(";");
  if (namearr) {
    for (Int_t i=0; i<namearr->GetEntries(); ++i) {
      TString cname(namearr->At(i)->GetName());
      if (cname.Length()<=0)
        continue;
      TClonesArray *arr = new TClonesArray("AliESDCaloCluster",0);
      arr->SetName(cname);
      fEvent->AddObject(arr);
    }
    delete namearr;
  }
  if (fDoTracks) {
    TClonesArray *arr = 0;
    if (fDoPicoTracks) {
      arr = new TClonesArray("AliPicoTrack",0);
      arr->SetName("Tracks");
    } else if (fDoMiniTracks) {
      arr = new TClonesArray("AliEsdTrackExt",0);
      arr->SetName("Tracks");
    } else {
      arr = new TClonesArray("AliESDtrackExt",0);
      arr->SetName("Tracks");
    }
    fEvent->AddObject(arr);
  }
  fEvent->GetStdContent();
  fEvent->WriteToTree(fTree);
  fTree->GetUserInfo()->Add(fEvent);
  PostData(1,fTree);
}
