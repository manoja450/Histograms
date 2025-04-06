#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>

void plotCh21_Areas(const char* filename) {
    // Open the input file
    TFile* file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    // Get the tree
    TTree* tree = (TTree*)file->Get("tree");
    if (!tree) {
        std::cerr << "Error: Could not find tree 'tree' in file" << std::endl;
        file->Close();
        return;
    }

    // Create histograms
    TH1F* hLow = new TH1F("hLow", "Channel 21 Areas;Area [ADC];Counts/20 ADC", 
                         150, 0, 3000);
    TH1F* hAll = new TH1F("hAll", "Channel 21 Areas ;Area [ADC];Counts/20 ADC",
                         150, 0, 3000);

    // Style settings
    hLow->SetLineColor(kBlue);
    hAll->SetLineColor(kRed);
    hLow->SetLineWidth(2);
    hAll->SetLineWidth(2);
    hLow->SetFillStyle(3003);  // Semi-transparent fill for low areas
    hAll->SetFillStyle(0);     // No fill for all areas
    hLow->SetFillColor(kBlue);
    hAll->SetFillColor(kRed);

    // Variables to read from tree
    Double_t area[23];
    Int_t triggerBits;
    tree->SetBranchAddress("area", area);
    tree->SetBranchAddress("triggerBits", &triggerBits);

    // Event loop
    Long64_t nEntries = tree->GetEntries();
    std::cout << "Processing " << nEntries << " events for Channel 21..." << std::endl;
    
    for (Long64_t entry = 0; entry < nEntries; entry++) {
        tree->GetEntry(entry);
        if (triggerBits != 34) continue;
        
        // Fill all areas histogram
        hAll->Fill(area[21]);
        
        // Fill low areas histogram
        if (area[21] <= 600) {
            hLow->Fill(area[21]);
        }
    }

    // Create canvas with proper margins
    TCanvas* c1 = new TCanvas("c1", "Channel 21 Area Distribution", 1200, 900);
    c1->SetLeftMargin(0.12);
    c1->SetRightMargin(0.08);
    c1->SetBottomMargin(0.12);
    c1->SetTopMargin(0.08);
    c1->SetGrid();
    c1->SetLogy();

    // Set explicit Y-axis range up to 10^6
    const double ymin = 0.5;    // Minimum value for log scale
    const double ymax = 1.0e6;  // Maximum value fixed at 10^6
    hAll->SetMinimum(ymin);
    hAll->SetMaximum(ymax);
    hLow->SetMinimum(ymin);
    hLow->SetMaximum(ymax);

    // Adjust histogram appearance
    hAll->SetStats(0);
    hLow->SetStats(0);
    hAll->GetYaxis()->SetTitleOffset(1.4);
    hAll->GetXaxis()->SetRangeUser(0, 3000);

    // Draw with proper scaling - all areas first as background
    hAll->Draw("HIST");
    hLow->Draw("HIST SAME");  // Overlay low areas

    // Create and position legend properly
    TLegend* leg = new TLegend(0.72, 0.78, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(hLow, "Areas ≤600 ADC", "f");
    leg->AddEntry(hAll, "All Areas", "l");
    leg->Draw();

    // Force update to apply all settings
    c1->Update();

    // Save output
    c1->SaveAs("Ch2_Area_Comparison.png");
    std::cout << "Saved plot to Ch2_Area_Comparison.png" << std::endl;

    // Cleanup
    delete leg;
    delete hLow;
    delete hAll;
    delete c1;
    file->Close();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.root>" << std::endl;
        return 1;
    }
    plotCh21_Areas(argv[1]);
    return 0;
}