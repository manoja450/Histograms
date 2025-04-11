//Pmts non log,SIPMs log plot
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include "TLatex.h"
#include <map>
#include <TSystem.h>
#include <sys/stat.h>
#include <TGaxis.h>
#include <TStyle.h>
#include <TLegend.h>

using namespace std;

void processLowLightEvents(const char *fileName) {
    // Create output directory
    const char* outDir = "area_plotswithThreshold";
    // Check if directory exists, if not create it
    struct stat info;
    if (stat(outDir, &info) != 0) {
        if (gSystem->mkdir(outDir, kTRUE) != 0) {
            cerr << "Error: Could not create directory " << outDir << endl;
            return;
        }
        cout << "Created directory: " << outDir << endl;
    } 
    else if (!(info.st_mode & S_IFDIR)) {
        cerr << "Error: " << outDir << " exists but is not a directory" << endl;
        return;
    }

    TFile *file = TFile::Open(fileName);
    if (!file || file->IsZombie()) {
        cerr << "Error opening file: " << fileName << endl;
        return;
    }

    TTree *tree = (TTree*)file->Get("tree");
    if (!tree) {
        cerr << "Error accessing TTree 'tree'!" << endl;
        file->Close();
        return;
    }

    Short_t adcVal[23][45];
    Double_t area[23];
    Int_t triggerBits;

    tree->SetBranchAddress("adcVal", adcVal);
    tree->SetBranchAddress("area", area);
    tree->SetBranchAddress("triggerBits", &triggerBits);

    Long64_t nEntries = tree->GetEntries();

    // PMT and SiPM channel mappings
    int pmtChannelMap[12] = {0, 10, 7, 2, 6, 3, 8, 9, 11, 4, 5, 1};
    int sipmChannelMap[10] = {12, 13, 14, 15, 16, 17, 18, 19, 20, 21};

    // Thresholds for PMTs and SiPMs
    double pmtThresholds[12] = {4800,6000,5000,6000,6000,4700,4500,3000,2000,5000,4500,4800};
    double sipmThresholds[10] = {800,800,1100,1200,550,600,650,450,600,650};

    // Create histograms
    TH1F *histPMT[12];         // Full histograms
    TH1F *histPMT_below[12];   // Below threshold
    TH1F *histSiPM[10];        // Full histograms
    TH1F *histSiPM_below[10];  // Below threshold
    
    for (int i = 0; i < 12; i++) {
        histPMT[i] = new TH1F(Form("PMT%d", i + 1), 
                             Form("PMT%d;Area;Events/500 ADC", i + 1),
                             100, 0, 50000);
        histPMT[i]->SetLineColor(kRed);
        histPMT[i]->SetMinimum(0); // Linear scale starts at 0
        
        histPMT_below[i] = new TH1F(Form("PMT%d", i + 1), 
                                   Form("PMT%d;Area;Events/500 ADC", i + 1),
                                   100, 0, 50000);
        histPMT_below[i]->SetLineColor(kBlue);
        histPMT_below[i]->SetFillColor(kBlue);
        histPMT_below[i]->SetFillStyle(3001);
    }
    
    for (int i = 0; i < 10; i++) {
        histSiPM[i] = new TH1F(Form("SiPM%d", i + 1), 
                              Form("SiPM%d;Area;Events/30 ADC", i + 1),
                              100, 0, 3000);
        histSiPM[i]->SetLineColor(kRed);
        histSiPM[i]->SetMinimum(0.1); // For log scale
        histSiPM[i]->SetMaximum(1e6); // For log scale
        
        histSiPM_below[i] = new TH1F(Form("SiPM%d", i + 1), 
                                    Form("SiPM%d ;Area;Events/30 ADC", i + 1),
                                    100, 0, 3000);
        histSiPM_below[i]->SetLineColor(kBlue);
        histSiPM_below[i]->SetFillColor(kBlue);
        histSiPM_below[i]->SetFillStyle(3001);
    }

    // Process each event
    for (Long64_t entry = 0; entry < nEntries; entry++) {
        tree->GetEntry(entry);
        if (triggerBits == 34) {
            for (int pmt = 0; pmt < 12; pmt++) {
                int adcIdx = pmtChannelMap[pmt];
                double a = area[adcIdx];
                histPMT[pmt]->Fill(a);
                if (a < pmtThresholds[pmt]) {
                    histPMT_below[pmt]->Fill(a);
                }
            }
            for (int sipm = 0; sipm < 10; sipm++) {
                int adcIdx = sipmChannelMap[sipm];
                double a = area[adcIdx];
                histSiPM[sipm]->Fill(a);
                if (a < sipmThresholds[sipm]) {
                    histSiPM_below[sipm]->Fill(a);
                }
            }
        }
    }

    // Save individual PMT plots (linear scale)
    TCanvas *individualCanvas = new TCanvas("IndividualCanvas", "Individual Plots", 1200, 900);
    for (int i = 0; i < 12; i++) {
        individualCanvas->Clear();
        gPad->SetLeftMargin(0.15);
        gPad->SetRightMargin(0.12);
        gPad->SetBottomMargin(0.15);
        gPad->SetTopMargin(0.12);
        
        // Linear scale for PMTs
        gPad->SetLogy(0);
        
        // Draw below threshold first for proper scaling
        histPMT_below[i]->Draw();
        histPMT[i]->Draw("same");
        
        
        histPMT[i]->GetXaxis()->SetTitleSize(0.06);
        histPMT[i]->GetYaxis()->SetTitleSize(0.06);
        histPMT[i]->GetYaxis()->SetTitleOffset(1.2);

        individualCanvas->SaveAs(Form("%s/PMT%d_area_linear.png", outDir, i + 1));
    
    }

    // Save individual SiPM plots (log scale)
    for (int i = 0; i < 10; i++) {
        individualCanvas->Clear();
        gPad->SetLeftMargin(0.15);
        gPad->SetRightMargin(0.12);
        gPad->SetBottomMargin(0.15);
        gPad->SetTopMargin(0.12);
        
        gPad->SetLogy();
        histSiPM[i]->Draw();
        histSiPM_below[i]->Draw("same");
        
        
        histSiPM[i]->GetXaxis()->SetTitleSize(0.06);
        histSiPM[i]->GetYaxis()->SetTitleSize(0.06);
        histSiPM[i]->GetYaxis()->SetTitleOffset(1.2);

        individualCanvas->SaveAs(Form("%s/SiPM%d_area_log.png", outDir, i + 1));

    }

    // Create master canvas
    TCanvas *masterCanvas = new TCanvas("MasterCanvas", "Combined PMT and SiPM Area Distributions", 3600, 3000);
    masterCanvas->Divide(5, 6, 0.005, 0.005);
    // Disable automatic histogram titles
gStyle->SetOptTitle(0);

    int layout[6][5] = {
        {-1,  -1,  20,  21, -1},
        {16,  9,   3,   7,  12},
        {15,  5,   4,   8,   -1},
        {19,  0,   6,   1,  17},
        {-1,  10,  11,  2,  13},
        {-1,  -1,  14,  18, -1}
    };

    map<int, int> pmtReverseMap;
    for (int i = 0; i < 12; i++) {
        pmtReverseMap[pmtChannelMap[i]] = i;
    }

    TGaxis::SetMaxDigits(3);
    gStyle->SetPaintTextFormat("4.1e");

    for (int row = 0; row < 6; row++) {
        for (int col = 0; col < 5; col++) {
            int padNum = row * 5 + col + 1;
            masterCanvas->cd(padNum);
            
            int channel = layout[row][col];
            if (channel == -1) {
                gPad->SetFillColor(0);
                gPad->Modified();
                gPad->Update();
                continue;
            }

            TH1F *hist = nullptr;
            TH1F *hist_below = nullptr;
            TString title;

            if (channel >= 0 && channel < 12) { // PMT
                auto it = pmtReverseMap.find(channel);
                if (it == pmtReverseMap.end()) continue;
                int pmtIdx = it->second;
                hist = histPMT[pmtIdx];
                hist_below = histPMT_below[pmtIdx];
                title = Form("PMT%d", pmtIdx + 1);
                gPad->SetLogy(0);
                hist_below->Draw();
                hist->Draw("same");
            } 
            else if (channel >= 12 && channel <= 21) { // SiPM
                int sipmIdx = channel - 12;
                if (sipmIdx < 0 || sipmIdx >= 10) continue;
                hist = histSiPM[sipmIdx];
                hist_below = histSiPM_below[sipmIdx];
                title = Form("SiPM%d", sipmIdx + 1);
                gPad->SetLogy();
                hist->Draw();
                hist_below->Draw("same");
            }

            hist->SetTitle("");
            hist->GetXaxis()->SetTitleSize(0.05);
            hist->GetYaxis()->SetTitleSize(0.07);
            hist->GetXaxis()->SetLabelSize(0.05);
            hist->GetYaxis()->SetLabelSize(0.05);
            hist->GetYaxis()->SetTitleOffset(1.2);
            hist->GetXaxis()->SetTitleOffset(1.1);
            hist->GetXaxis()->SetNdivisions(505);
            hist->GetYaxis()->SetNdivisions(505);

            gPad->SetLeftMargin(0.15);
            gPad->SetRightMargin(0.10);
            gPad->SetBottomMargin(0.12);
            gPad->SetTopMargin(0.12);

            TLatex *latex = new TLatex();
            latex->SetTextSize(0.12);
            latex->SetTextAlign(22);
            latex->SetNDC(true);
            latex->DrawLatex(0.5, 0.93, title);
        }
    }

    masterCanvas->cd();
    TLatex *textbox = new TLatex();
    textbox->SetTextSize(0.05);
    textbox->SetTextAlign(15);
    textbox->SetNDC(true);
    textbox->DrawLatex(0.01, 0.11, "X axis: Area");
    textbox->DrawLatex(0.01, 0.07, "Y axis: Events");

    masterCanvas->SaveAs(Form("%s/Combined_PMT_SiPM_Area_Distributions.png", outDir));
    
    // Clean up
    for (int i = 0; i < 12; i++) {
        delete histPMT[i];
        delete histPMT_below[i];
    }
    for (int i = 0; i < 10; i++) {
        delete histSiPM[i];
        delete histSiPM_below[i];
    }
    delete individualCanvas;
    delete masterCanvas;
    file->Close();

    cout << "All plots saved in directory: " << outDir << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <root_file>" << endl;
        return 1;
    }
    processLowLightEvents(argv[1]);
    return 0;
}
