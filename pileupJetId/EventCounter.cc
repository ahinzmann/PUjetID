#include "KKousour/TopAnalysis/plugins/EventCounter.h"//this needs to be changed

using namespace reco;

EventCounter::EventCounter(edm::ParameterSet const& cfg): 
  pupInfoToken(consumes<edm::View<PileupSummaryInfo> >(edm::InputTag("slimmedAddPileupInfo"))),
  genEvtInfoToken(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
  genParticlesToken(consumes<edm::View<reco::GenParticle> >(edm::InputTag("prunedGenParticles")))
{
}
//////////////////////////////////////////////////////////////////////////////////////////
void EventCounter::beginJob() 
{
  evtWtHisto_       = fs_->make<TH1F>("GenEventWeight","GenEventWeight",1,-0.5,0.5);
  puHisto_          = fs_->make<TH1F>("pileup","pileup",40,0,40);
  decayHisto_       = fs_->make<TH1F>("decay","decay",4,-1,3);
  h_ptTopParton_[0] = fs_->make<TH1F>("h_ptTopParton_0","h_ptTopParton_0",1500,0,1500);
  h_ptTopParton_[1] = fs_->make<TH1F>("h_ptTopParton_1","h_ptTopParton_1",1500,0,1500);
  h_yTopParton_[0]  = fs_->make<TH1F>("h_yTopParton_0" ,"h_yTopParton_0" ,600,-3,3);
  h_yTopParton_[1]  = fs_->make<TH1F>("h_yTopParton_1" ,"h_yTopParton_1" ,600,-3,3);
  h_mTTbarParton_   = fs_->make<TH1F>("h_mTTbarParton" ,"h_mTTbarParton" ,3000,0,3000);
  h_yTTbarParton_   = fs_->make<TH1F>("h_yTTbarParton" ,"h_yTTbarParton" ,600,-3,3); 
  h_ptTTbarParton_  = fs_->make<TH1F>("h_ptTTbarParton","h_ptTTbarParton",1000,0,1000);
}
//////////////////////////////////////////////////////////////////////////////////////////
void EventCounter::endJob() 
{  
}
//////////////////////////////////////////////////////////////////////////////////////////
void EventCounter::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup) 
{
  int npu(0);
  if (!iEvent.isRealData()) { 
    iEvent.getByToken(genEvtInfoToken,genEvtInfo);
    iEvent.getByToken(pupInfoToken,pupInfo);
    iEvent.getByToken(genParticlesToken,genParticles);

    evtWtHisto_->Fill(0.0,genEvtInfo->weight());
    edm::View<PileupSummaryInfo>::const_iterator PUI;

    LorentzVector p4T(0,0,0,0),p4Tbar(0,0,0,0);
    bool WPlusLep(false),WMinusLep(false);

    for(unsigned ip = 0; ip < genParticles->size(); ++ ip) {
      const GenParticle &p = (*genParticles)[ip];
      if (p.pdgId() == 24) {
        for(unsigned k = 0; k < p.numberOfDaughters(); k++) {
          int daughterID = p.daughter(k)->pdgId();
          if (daughterID == -11 || daughterID == -13 || daughterID == -15) {
            WPlusLep = true;
          }
        }
      }
      if (p.pdgId() == -24) {
        for(unsigned k = 0; k < p.numberOfDaughters(); k++) {
          int daughterID = p.daughter(k)->pdgId();
          if (daughterID == 11 || daughterID == 13 || daughterID == 15) {
            WMinusLep = true;
          }
        }
      }
      if (fabs(p.pdgId()) == 6 && p.statusFlags().isFirstCopy() && p.isHardProcess()) {
        if (p.pdgId() == 6) {
          p4T = p.p4();
        }
        if (p.pdgId() == -6) {
          p4Tbar = p.p4();
        }
      } 
    }// end of particle loop

    h_ptTopParton_[0]->Fill(TMath::Max(p4T.pt(),p4Tbar.pt()));
    h_ptTopParton_[1]->Fill(TMath::Min(p4T.pt(),p4Tbar.pt()));
    if (p4T.pt() > p4Tbar.pt()) {
      h_yTopParton_[0]->Fill(p4T.Rapidity());
      h_yTopParton_[1]->Fill(p4Tbar.Rapidity());
    }
    else {
      h_yTopParton_[1]->Fill(p4T.Rapidity());
      h_yTopParton_[0]->Fill(p4Tbar.Rapidity()); 
    }
    h_mTTbarParton_->Fill((p4T+p4Tbar).mass());
    h_yTTbarParton_->Fill((p4T+p4Tbar).Rapidity());
    h_ptTTbarParton_->Fill((p4T+p4Tbar).pt());

    int decay(-1);

    if (WPlusLep && WMinusLep)   decay = 2;
    if (WPlusLep && !WMinusLep)  decay = 1;
    if (!WPlusLep && WMinusLep)  decay = 1;
    if (!WPlusLep && !WMinusLep) decay = 0;

    decayHisto_->Fill(decay);

    for(PUI = pupInfo->begin(); PUI != pupInfo->end(); ++PUI) {
      if (PUI->getBunchCrossing() == 0) {
        npu = PUI->getTrueNumInteractions();
      }
    }
    puHisto_->Fill(npu);
    
  }
  else {
    evtWtHisto_->Fill(0.0,1);
    puHisto_->Fill(0.0);
    h_ptTopParton_[0]->Fill(-999.0);
    h_ptTopParton_[1]->Fill(-999.0);
    h_yTopParton_[0]->Fill(-999.0);
    h_yTopParton_[1]->Fill(-999.0);
    h_mTTbarParton_->Fill(-999.0);
    h_yTTbarParton_->Fill(-999.0);
    h_ptTTbarParton_->Fill(-999.0);
  }
}
//////////////////////////////////////////////////////////////////////////////////////////
EventCounter::~EventCounter() 
{
}

DEFINE_FWK_MODULE(EventCounter);
