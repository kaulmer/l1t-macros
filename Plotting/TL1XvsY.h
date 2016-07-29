#ifndef TL1XVSY_H
#define TL1XVSY_H

#include <string>
#include <vector>
#include <sstream>

#include <TFile.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TLine.h>

#include "TL1Plots.h"

class TL1XvsY : public TL1Plots
{
    public:
        ~TL1XvsY();

        virtual void InitPlots();
        virtual void OverwritePlots();
        virtual void Fill(const double & xVal, const double & yVal, const int & pu=0);
        virtual void DrawPlots();
        void DrawCmsStamp();

        void SetXBins(const std::vector<double> & xBins);
        void SetX(const std::string & xName, const std::string & xTitle);
        void SetYBins(const std::vector<double> & yBins);
        void SetY(const std::string & yName, const std::string & yTitle);
    private:
        std::vector<TH2F*> fPlot;
        TFile * fRootFile;

        std::string fXName, fXTitle;
        std::string fYName, fYTitle;
        std::vector<double> fXBins, fYBins;

};

TL1XvsY::~TL1XvsY()
{
    delete fRootFile;
}

void TL1XvsY::InitPlots()
{
    fRootFile = TFile::Open(Form("%s/xy_%s.root",this->GetOutDir().c_str(),this->GetOutName().c_str()),"RECREATE");
    fPlot.emplace_back(new TH2F(Form("xy_%s_vs_%s",fXName.c_str(),fYName.c_str()),"", fXBins.size()-1,&(fXBins)[0], fYBins.size()-1,&(fYBins)[0]));
    fPlot.back()->SetDirectory(0);
    fPlot.back()->Sumw2();
    fPlot.back()->GetXaxis()->SetTitle(fXTitle.c_str());
    fPlot.back()->GetYaxis()->SetTitle(fYTitle.c_str());
    for(int ipu=0; ipu<this->GetPuType().size(); ++ipu)
    {
        fPlot.emplace_back(new TH2F(Form("xy_%s_vs_%s_%s",fXName.c_str(),fYName.c_str(), this->GetPuType()[ipu].c_str()),"", fXBins.size()-1,&(fXBins)[0], fYBins.size()-1,&(fYBins)[0]));
        fPlot.back()->SetDirectory(0);
        fPlot.back()->Sumw2();
        fPlot.back()->GetXaxis()->SetTitle(fXTitle.c_str());
        fPlot.back()->GetYaxis()->SetTitle(fYTitle.c_str());
    }
}

void TL1XvsY::OverwritePlots()
{
    fPlot.clear();
    TFile * rootFile = TFile::Open(this->GetOverwriteRootFilename().c_str(),"READ");

    fRootFile = TFile::Open(Form("%s/xy_%s_overwrite.root",this->GetOutDir().c_str(),this->GetOutName().c_str()),"RECREATE");
    fPlot.push_back((TH1F*)rootFile->Get(this->GetOverwriteHistname().c_str()));
    fPlot.back()->SetDirectory(0);
    fPlot.back()->GetXaxis()->SetTitle(fXTitle.c_str());
    fPlot.back()->GetYaxis()->SetTitle(fYTitle.c_str());

    for(int ipu=0; ipu<this->GetPuType().size(); ++ipu)
    {
        fPlot.push_back((TH1F*)rootFile->Get("%s_%s",this->GetOverwriteHistname().c_str(),this->GetPuType()[ipu].c_str()));
        fPlot.back()->SetDirectory(0);
        fPlot.back()->GetXaxis()->SetTitle(fXTitle.c_str());
        fPlot.back()->GetYaxis()->SetTitle(fYTitle.c_str());
    }
    delete rootFile;
}

void TL1XvsY::Fill(const double & xVal, const double & yVal, const int & pu=0)
{
    fPlot[0]->Fill(xVal,yVal,this->GetPuWeight(pu));
    for(int ipu=0; ipu<this->GetPuType().size(); ++ipu)
    {
        if( pu >= this->GetPuBins()[ipu] && pu < this->GetPuBins()[ipu+1] )
            fPlot[ipu+1]->Fill(xVal,yVal,this->GetPuWeight(pu));
    }
}

void TL1XvsY::DrawPlots()
{
    TCanvas * can(new TCanvas(Form("can_%f",this->GetRnd()),"")); 
    fPlot[0]->SetMinimum(1);
    fPlot[0]->Draw("colz");
    fRootFile->WriteTObject(fPlot[0]);
    can->SetLogz();

    DrawCmsStamp();
    TLatex * nomlatex = new TLatex();
    nomlatex->SetNDC();
    nomlatex->SetTextFont(42);
    nomlatex->SetTextAlign(31);
    //nomlatex->DrawLatex(0.82,0.15,"<PU>=14");

    std::string outName = Form("%s/xy_%s.pdf",this->GetOutDir().c_str(),this->GetOutName().c_str());
    can->SaveAs(outName.c_str());

    for(int ipu=0; ipu<this->GetPuType().size(); ++ipu)
    {
        TCanvas * can2(new TCanvas(Form("can_%f",this->GetRnd()),""));
        fPlot[ipu+1]->SetMinimum(1);
        fPlot[ipu+1]->Draw("colz");
        fRootFile->WriteTObject(fPlot[ipu+1]);
        can2->SetLogz();

        DrawCmsStamp();
        TLatex * latex1(new TLatex());
        latex1->SetNDC();
        latex1->SetTextFont(42);
        latex1->SetTextAlign(31);
        std::stringstream entryName;
        if( ipu<this->GetPuType().size()-1 ) entryName << this->GetPuBins()[ipu] << " #leq PU < " << this->GetPuBins()[ipu+1];
        else entryName << this->GetPuBins()[ipu] << " #leq PU";
        latex1->DrawLatex(0.84,0.16,entryName.str().c_str());
        entryName.str("");

        outName = Form("%s/xy_%s_%s.pdf",this->GetOutDir().c_str(),this->GetOutName().c_str(),this->GetPuType()[ipu].c_str());
        can2->SaveAs(outName.c_str());
    }
}

void TL1XvsY::DrawCmsStamp()
{
    TLatex * latex(new TLatex());
    latex->SetNDC();
    latex->SetTextFont(42);
    //latex->SetTextAlign(11);
    if( this->GetSampleName() == "Data" )
        latex->DrawLatex(0.15,0.92,Form("#bf{CMS} #it{Preliminary} %s",this->GetSampleTitle().c_str()));
    else
        latex->DrawLatex(0.15,0.92,Form("#bf{CMS} #it{Simulation Preliminary} %s",this->GetSampleTitle().c_str()));
    latex->SetTextAlign(31);
    latex->DrawLatex(0.92,0.92,Form("%s (13 TeV)",this->GetRun().c_str()));
    latex->SetTextAlign(32);
    latex->DrawLatex(0.82,0.25,this->GetAddMark().c_str());

    TLine * line(new TLine());
    line->SetLineStyle(7);
    line->SetLineWidth(3);
    line->DrawLine(fXBins.front(),fYBins.front(),fXBins.back(),fYBins.back());
}

void TL1XvsY::SetXBins(const std::vector<double> & xBins)
{
    fXBins = xBins;
}

void TL1XvsY::SetX(const std::string & xName, const std::string & xTitle)
{
    fXName = xName;
    fXTitle = xTitle;
}

void TL1XvsY::SetYBins(const std::vector<double> & yBins)
{
    fYBins = yBins;
}

void TL1XvsY::SetY(const std::string & yName, const std::string & yTitle)
{
    fYName = yName;
    fYTitle = yTitle;
}

#endif
