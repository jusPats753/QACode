// Macro to plot overlay histograms from multiple data runs

/*
   MACRO OVERVIEW:
   This macro is designed to overlay histograms from various data runs for sPHENIX EMCal QA.
   The histograms are extracted from ROOT files corresponding to different runs.
   Once extracted, the histograms are overlaid onto a single canvas for comparison.
   The OverlayPlotter class manages the loading, overlaying, and display of these histograms.
*/

#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <TColor.h>
#include <TLegend.h>

class OverlayPlotter {
public:
    
    // Struct to hold run-specific data
    struct RunData {
        Color_t color;     // Color for this run's histogram
        int sebCount;      // Count of Sub Event Buffers (SEBs) for each run
    };
    
    // Constructor to initialize canvas, legend, and other attributes
    OverlayPlotter(bool normalize) : normalize_(normalize) {
        // Initializing the canvas
        cOverlay_ = new TCanvas("cOverlay", "", 800, 600);
        cOverlay_->SetLogy();
        cOverlay_->SetGrid();
        
        // Initializing the legend
        leg_ = new TLegend(0.6, 0.6, 0.9, 0.9);
        leg_->SetNColumns(2);
        leg_->SetFillColorAlpha(0, 0.2);
        leg_->SetBorderSize(1);
        leg_->SetMargin(0.15);
        leg_->SetTextSize(0.025);
        
        for (const auto& entry : runDataMap_) {
            runNumbers_.push_back(entry.first);
        }
    }
    
    // Resetting the canvas and legend for a fresh plot
    void ResetCanvas() {
        cOverlay_->Clear();
        leg_->Clear();
    }
    
    // Primary function to overlay histograms for all runs
    void Overlay(const std::string& histName, const std::string& title, const std::string& xAxisTitle, const std::string& yAxisTitle) {
        ResetCanvas();
        
        // Loop over all specified run numbers
        for (const auto& run : runNumbers_) {
            OverlayRun(run, histName, title, xAxisTitle, yAxisTitle);
        }
        
        // Drawing the legend and other labels
        leg_->Draw();
        TLatex sphenixLabel;
        sphenixLabel.SetTextSize(0.03);
        sphenixLabel.DrawLatexNDC(0.67, 0.575, "sPHENIX EMCal QA");
        cOverlay_->SetTitle(title.c_str());
        cOverlay_->Update();
        cOverlay_->SaveAs(("/Users/patsfan753/Desktop/QA_EMCal/OverlayedPlotOutput/Overlayed_" + histName + "_QA_October.png").c_str());
    }


private:
    // Attributes for the class
    bool normalize_;
    TCanvas* cOverlay_;
    TLegend* leg_;
    std::unordered_map<std::string, RunData> runDataMap_ = {
        // (Run Number: {Color, SEB Count})
        {"21813", {kBlue, 7}},
        {"21796", {kOrange+7, 8}},
        {"21615", {kBlack, 8}},
        {"21599", {kBlue+3, 8}},
        {"21598", {kRed, 8}},
        {"21891", {kCyan+3, 7}},
        {"22979", {kMagenta, 5}},
        {"22950", {kViolet+1, 5}},
        {"22949", {kMagenta+2, 5}},
        {"22951", {kAzure+4, 5}},
        {"22982", {kAzure+2, 5}},
        {"21518", {kPink-3, 8}},
        {"21520", {kOrange+1, 8}},
        {"21889", {kGray+1, 7}}
    };
    
    //List of run numbers
    std::vector<std::string> runNumbers_;
    
    // Function to overlay the histogram data for a specific run.
    void OverlayRun(const std::string& run,        // The specific run number to process.
                    const std::string& histName,   // The name of the histogram to overlay.
                    const std::string& title,      // Title for the histogram.
                    const std::string& xAxisTitle, // Title for the X-axis.
                    const std::string& yAxisTitle) // Title for the Y-axis.
    {
        // Base directory where the root output files are stored.
        const char *baseDir = "/Users/patsfan753/Desktop/QA_EMCal/rootOutput/";
        // Constructs the full file path using the base directory and the run number.
        std::string filePath = baseDir + run + "/qa.root";
        
        // Notify the user about the run currently being processed.
        std::cout << "\nProcessing data for run: " << run << std::endl;
        
        
        // Open the ROOT file for the specified run.
        TFile *file = TFile::Open(filePath.c_str());
        
        // Check if the file opened correctly and is not corrupted.
        if (!file || file->IsZombie()) {
            std::cout << "File issue for run: " << run << std::endl;
            return;
        }
        
        // Extract the specified histogram from the file.
        TH1F *hist = (TH1F*)file->Get(histName.c_str());
        // Print the name of the histogram being accessed
        std::cout << "Accessing histogram: " << histName << std::endl;
        
        // Extract another histogram named "hNClusters" from the file.
        TH1F *hNClusters = (TH1F*)file->Get("hNClusters");
        // Check for potential issues with the histograms.
        if (!hist || !hNClusters) {
            std::cout << "Histogram issue for run: " << run << std::endl;
            return;
        }
        
        // Get the color assigned to the current run from the map.
        Color_t color = runDataMap_[run].color;
        std::cout << "Color assigned: " << color << std::endl;
        std::cout << "SEB count for the run: " << runDataMap_[run].sebCount << std::endl;
        
        // Disable the statistics box for the histogram since doesnt provide information for overlayed plots
        hist->SetStats(kFALSE);
        int nEvents = 0;
        int nSEBs = 0;
        
        // If normalization is enabled, scale the histogram based on the number of events and SEBs.
        if (normalize_) {
            nEvents = hNClusters->GetEntries();
            nSEBs = runDataMap_[run].sebCount;
            Normalize(hist, nEvents, nSEBs);
        }
        // Styling for the histogram.
        hist->SetLineWidth(1);  // Setting the line width to half the default width

        hist->SetMarkerStyle(20);
        hist->SetMarkerSize(0.6);
        hist->SetMarkerColor(color);
        hist->SetLineColor(color);
        hist->SetTitle(title.c_str());
        hist->GetXaxis()->SetTitle(xAxisTitle.c_str());
        hist->GetYaxis()->SetTitle(yAxisTitle.c_str());
        
        // Bold the X-axis and Y-axis titles.
        hist->GetXaxis()->SetTitleFont(42);  // 42 corresponds to bold in ROOT
        hist->GetYaxis()->SetTitleFont(42);  // 42 corresponds to bold in ROOT

        // Adjust the position of the Y-axis title.
        hist->GetYaxis()->SetTitleOffset(1.4);
        
        // Draw the histogram. If it's the first run, draw a fresh plot; otherwise, overlay on the existing plot.
        if (run == *runNumbers_.begin()) {
            hist->Draw("HIST");
        } else {
            hist->Draw("HIST SAME");
        }
        
        // Add the histogram to the legend with the run number.
        leg_->AddEntry(hist, Form("Run: %s", run.c_str()), "l");
        std::cout << "Completed for run: " << run << std::endl;
        
    }
    
    // Function to normalize the histogram by the number of events and SEBs.
    void Normalize(TH1F* h,        // The histogram to normalize.
                   int nEvents,    // The number of events for the specific run.
                   int nSEBs)      // The number of SEBs for the specific run.
    {
        // Normalize the histogram if both numbers are greater than zero.
        if (nEvents > 0 && nSEBs > 0) {
            h->Scale(1.0 / (nEvents * nSEBs));
        }
    }
};

// Main function to generate overlay plots for various histograms.
void OverlayedPlotGenerator() {
    // Create an instance of the OverlayPlotter class with normalization enabled.
    OverlayPlotter overlayPlotter(true);
    
    // Generate overlay plots for different types of histograms with their respective titles
    
    // Chi2 Plot
    overlayPlotter.Overlay(
        "hClusterChi",
        "Cluster #chi^{2} Distribution",
        "Cluster #chi^{2}",
        "Counts"
    );

    // MBD Charge Plot
    overlayPlotter.Overlay(
        "hTotalMBD",
        "MBD Charge Distribution",
        "MBD Charge",
        "Counts"
    );

    // Cluster pT Plot
    overlayPlotter.Overlay(
        "hClusterPt",
        "Cluster p_{T} Good Runs Distribution",
        "Cluster p_{T} (GeV)",
        "Counts"
    );

    // Cluster Energy Plot
    overlayPlotter.Overlay(
        "hTotalCaloE",
        "Total Calorimeter Energy Distribution",
        "Cluster Energy (GeV)",
        "Counts"
    );

    // Cluster ECore Plot
    overlayPlotter.Overlay(
        "hClusterECore",
        "Cluster ECore Distribution",
        "Cluster ECore (GeV)",
        "Counts"
    );
}
