#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <iostream>
#include <unordered_map>
#include <TROOT.h>

class SinglePlotter {
public:
    // Class constructor: initializes member variables based on user input
    SinglePlotter(bool normalize, bool applyCut = false, double cutValue = 0.0, const std::vector<std::string>& histToCut = {})
        : normalize_(normalize), applyCut_(applyCut), cutValue_(cutValue), histToCut_(histToCut) {}
    
    // Method to plot histograms
    // This function will loop through every run and make plots
    void Plot(const std::string& histName, const std::string& title, const std::string& xAxisTitle, const std::string& yAxisTitle) 
    {
        // Header for console output
        std::cout << "===========================================" << std::endl;
        std::cout << "Start Plotting for Histogram: " << histName << std::endl;
        std::cout << "===========================================" << std::endl;
        
        // Loop through each run and its SEB (Sub-Event Buffer) count
        for (const auto& runEntry : runDataMap_) {
                const std::string& run = runEntry.first;
                int sebCount = runEntry.second;
                
                // Console output for each run
                std::cout << "-------------------------------------------------" << std::endl;
                std::cout << "| Processing Run: " << run << std::endl;
                std::cout << "| SEB Count: " << sebCount << std::endl;
            
                // Actual plotting method
                PlotRun(run, histName, title, xAxisTitle, yAxisTitle, sebCount);
            
                // Console output post-processing for each run
                std::cout << "-------------------------------------------------" << std::endl;
            }
            std::cout << "Completed Plotting for Histogram: " << histName << std::endl << std::endl;
        }

private:
    // Private variables to hold settings
    bool normalize_;    // Whether to normalize histogram or not
    bool applyCut_;     // Whether to apply energy cut or not
    double cutValue_;   // Cut value for energy
    std::vector<std::string> histToCut_;    // List of histograms to apply cut
    
    // Map containing run numbers and corresponding SEB (Sub-Event Buffer) counts
    std::unordered_map<std::string, int> runDataMap_ = {
        // Run numbers are keys and SEB counts are values
        {"21813", 7},
        {"21796", 8},
        {"21615", 8},
        {"21599", 8},
        {"21598", 8},
        {"21891", 7},
        {"22979", 5},
        {"22950", 5},
        {"22949", 5},
        {"22951", 5},
        {"22982", 5},
        {"21518", 8},
        {"21520", 8},
        {"21889", 7}
    };
    
    // Helper function to get the output path for each histogram
    std::string GetOutputPath(const std::string& histName)
    {
        /*
         CREATE SEPERATE FOLDER FOR EACH OUTPUT, SET THE PATH, the code will do the work from there
         */
        
        // Map histogram name to output directory
        if (histName == "hClusterChi") return "/Users/patsfan753/Desktop/QA_EMCal/Individual_Plot_Output/Cluster_Chi/";
        if (histName == "hClusterPt") return "/Users/patsfan753/Desktop/QA_EMCal/Individual_Plot_Output/Cluster_pt/";
        if (histName == "hClusterECore") return "/Users/patsfan753/Desktop/QA_EMCal/Individual_Plot_Output/ECore/";
        if (histName == "hTotalCaloE") return "/Users/patsfan753/Desktop/QA_EMCal/Individual_Plot_Output/Total_Calo_Energy/";
        if (histName == "hTotalMBD") return "/Users/patsfan753/Desktop/QA_EMCal/Individual_Plot_Output/MBD_charge/";
        return "";
    }
    
    // Helper function to apply energy cut
    void ApplyEnergyCut(TH1F* hist) {
        std::cout << "| Applying Energy Cut..." << std::endl;
        
        // Loop over each bin
        for (int bin = 1; bin <= hist->GetNbinsX(); bin++) {
            if (hist->GetBinCenter(bin) < cutValue_) {
                hist->SetBinContent(bin, 0);  // set the bin content to zero below the cut value
            }
        }
        std::cout << "| Energy Cut Applied for bins below " << cutValue_ << " GeV" << std::endl;
    }
    
    // Method to plot a single run
    void PlotRun(const std::string& run, const std::string& histName, const std::string& title, const std::string& xAxisTitle, const std::string& yAxisTitle, int sebCount)
    {
        
        /*
         ENSURE TO CHANGE PATH TO ROOT FILES HERE
         */
        
        // Directory where ROOT files are located
        const char *baseDir = "/Users/patsfan753/Desktop/QA_EMCal/rootOutput/";
        std::string filePath = baseDir + run + "/qa.root";  // Construct the full file path
        
        TFile *file = TFile::Open(filePath.c_str());
        if (!file || file->IsZombie()) {
            std::cout << "File issue for run: " << run << std::endl;
            return;
        }
        
        // Open ROOT file for the given run
        TH1F *hist = (TH1F*)file->Get(histName.c_str());
        
        // Extract histogram of number of clusters for normalization
        TH1F *hNClusters = (TH1F*)file->Get("hNClusters");
        if (!hist || !hNClusters) {
            std::cout << "Histogram issue for run: " << run << std::endl;
            return;
        }

        // Apply normalization if set to true
        if (normalize_) {
            //get number of events from number of clusters histogram (filled once per event)
            int nEvents = hNClusters->GetEntries();
            Normalize(hist, nEvents, sebCount);
            std::cout << "| Normalized using nEvents: " << nEvents << " and SEB count: " << sebCount << std::endl;
        } else {
            std::cout << "| No normalization applied." << std::endl;
        }
        
        // Apply energy cut if necessary
        if (applyCut_ && std::find(histToCut_.begin(), histToCut_.end(), histName) != histToCut_.end()) {
            ApplyEnergyCut(hist);
        } else {
            std::cout << "| No energy cut applied for histogram: " << histName << std::endl;
        }
        
        // Create and save canvas
        TCanvas* c = new TCanvas(Form("c_%s_%s", histName.c_str(), run.c_str()), "", 800, 600);
        c->SetLogy();
        hist->SetLineWidth(2);
        
        // Set plot titles and labels
        hist->SetTitle(Form("%s (Run: %s)", title.c_str(), run.c_str()));
        hist->GetXaxis()->SetTitle(xAxisTitle.c_str());
        hist->GetYaxis()->SetTitle(yAxisTitle.c_str());
        hist->Draw("HIST");
        c->SaveAs(Form("%s%s_Run_%s.png", GetOutputPath(histName).c_str(), histName.c_str(), run.c_str()));
        std::cout << "| Saved plot for histogram: " << histName << " and run: " << run << " at path: " << Form("%s%s_Run_%s.png", GetOutputPath(histName).c_str(), histName.c_str(), run.c_str()) << std::endl;
        
        // Cleanup
        delete c;
        file->Close();
        delete file;
    }
    
    // Function to normalize histogram based on number of events and SEB numbers
    void Normalize(TH1F* h, int nEvents, int nSEBs) {
        if (nEvents > 0 && nSEBs > 0) {
            h->Scale(1.0 / (nEvents * nSEBs));
        }
    }
};

std::vector<std::string> AskHistogramToCut() {
    // Declare an array containing the names of histograms that can be chosen for applying an energy cut.
    const char *options[] = {"hClusterChi", "hClusterPt", "hClusterECore", "hTotalCaloE", "hTotalMBD"};
    // Vector to store the histograms that the user wants to apply the energy cut to.
    std::vector<std::string> histogramsToCut;
    
    // Prompt the user to choose histograms for applying the energy cut.
    std::cout << "Which histograms would you like to apply the energy cut to? (Separate choices by spaces, e.g., '1 3 4')" << std::endl;
    
    // Loop through the 'options' array and display the available histograms.
    for (int i = 0; i < 5; ++i) {
        std::cout << i + 1 << ". " << options[i] << std::endl;
    }
    
    // Declare a string 'line' to store the input from the user.
    std::string line;
    std::getline(std::cin, line);  // To clear any previous inputs
    std::getline(std::cin, line);  // Get the actual input
    
    // Use stringstream to break the user's line of input into separate integers.
    std::stringstream ss(line);

    int choice; // Declare an integer to store each individual choice.
    
    // Loop through the stringstream to extract individual integers.
    while (ss >> choice) {
        // Check if the choice is a valid number between 1 and 5.
        if (choice >= 1 && choice <= 5) {
            histogramsToCut.push_back(options[choice - 1]);  // adjust for 0-indexing
        } else {
            std::cout << "Invalid choice: " << choice << ". Please select numbers between 1 and 5." << std::endl;
        }
    }
    // Return the vector containing histograms chosen for applying the energy cut.
    return histogramsToCut;
}


double AskEnergyCutValue() {
    // Declare a variable 'energy' to store the energy cut value entered by the user.
    double energy;

    // Start an infinite loop to ensure that the user inputs a valid value.
    while (true) {
        // Prompt the user to enter the energy cut value.
        std::cout << "Enter the energy cut value (in GeV, numeric values only): ";
        std::cin >> energy;  // Read the input from the user.

        // Check if the input operation failed (e.g., if a non-numeric value was entered).
        if (!std::cin.fail()) {
            break;  // Exit the loop if the input was valid.
        }
        
        // Clear the error flags of the input stream.
        std::cin.clear();
        
        // Ignore characters up to the next newline character to prepare for the next input operation.
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        // Display an error message to the user.
        std::cout << "Invalid input. Please enter a numeric value for the energy cut." << std::endl;
    }
    // Return the valid energy cut value.
    return energy;
}


bool AskApplyCut() {
    // Declare a string 'response' to store the user's answer.
    std::string response;

    // Prompt the user to decide whether or not to apply a minimum cluster energy cut.
    std::cout << "Would you like to apply a minimum cluster energy cut? (yes/no): ";
    
    // Start an infinite loop to ensure that the user inputs a valid response.
    while (true) {
        std::cin >> response;  // Read the input from the user.

        // Check if the response is "yes" or "no".
        if (response == "yes") {
            return true;  // Return true if the user wants to apply the energy cut.
        } else if (response == "no") {
            return false;  // Return false if the user does not want to apply the energy cut.
        } else {
            // Display an error message if the input was neither "yes" nor "no".
            std::cout << "Invalid response. Please answer 'yes' or 'no': ";
        }
    }
}


void SinglePlotGenerator() {
    // Call AskApplyCut() to determine if an energy cut should be applied.
    bool applyCut = AskApplyCut();
    
    // Declare a vector to store the histogram names to which the cut will be applied.
    std::vector<std::string> histToCut;

    // Declare a variable to store the energy cut value.
    double cutValue = 0;

    // If an energy cut should be applied, get further details.
    if (applyCut) {
        histToCut = AskHistogramToCut(); // Get the list of histograms for the cut.
        cutValue = AskEnergyCutValue();  // Get the energy cut value.
    }
    
    // Create a SinglePlotter object and initialize it with relevant parameters.
    SinglePlotter plotter(true, applyCut, cutValue, histToCut);

    // Plot various histograms using the Plot method from the SinglePlotter object.

    // Chi^2 Plot
    plotter.Plot(
        "hClusterChi",                      // Histogram name
        "Cluster #chi^{2} Distribution",    // Title
        "Cluster #chi^{2}",                 // X-axis label
        "Counts"                            // Y-axis label
    );

    // MBD Charge Plot
    plotter.Plot(
        "hTotalMBD",                   // Histogram name
        "MBD Charge Distribution",     // Title
        "MBD Charge",                  // X-axis label
        "Counts"                       // Y-axis label
    );

    // Cluster pT Plot
    plotter.Plot(
        "hClusterPt",                          // Histogram name
        "Cluster p_{T} Good Runs Distribution", // Title
        "Cluster p_{T} (GeV)",                  // X-axis label
        "Counts"                                // Y-axis label
    );

    // Cluster Energy Plot
    plotter.Plot(
        "hTotalCaloE",                   // Histogram name
        "Total Calorimeter Energy Distribution", // Title
        "Cluster Energy (GeV)",          // X-axis label
        "Counts"                         // Y-axis label
    );

    // Cluster ECore Plot
    plotter.Plot(
        "hClusterECore",               // Histogram name
        "Cluster ECore Distribution",  // Title
        "Cluster ECore (GeV)",         // X-axis label
        "Counts"                       // Y-axis label
    );
}
