
/*******************************************************************
 *                                                                 *
 * Date: 02.01.2009     Author: Irakli                             *
 *                                                                 *
 * This macro is used for the Pi0 invariant mass based energy      *
 *                                                                 *
 * calibbration of the Crystal Ball detector.                      *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

#include "iCalibPIDphi.hh"

ClassImp(iCalibPIDphi)

//_____________________________________________
iCalibPIDphi::iCalibPIDphi()
{
    this->Init();

    if (gROOT->GetFile())
        histofile = gFile;
    else
        histofile = TFile::Open(strPIDHistoFile);

    printf("\n ---------------------------------------- \n");
    printf(" Read Histo File: \"%s\"\n", histofile->GetName());

    if (!strPIDHistoName.Length())
        hCBPhiVsPID = (TH2F*) histofile->Get("iCBPhiVsPIDID_cut");
    else
        hCBPhiVsPID = (TH2F*) histofile->Get(strPIDHistoName.Data());

    if (!hCBPhiVsPID)
    {
        printf("\n ERROR: The histogram named: %s can not be found!!!\n"
               "          Check in \"config.cfg\" file PID.HName !\n", strPIDHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n", strPIDHistoName.Data());

    this->InitGUI();

    this->DoFor(1);
}

//_____________________________________________
iCalibPIDphi::iCalibPIDphi(TH2F* h1)
{

    this->Init();

    hCBPhiVsPID = (TH2F*) h1;

    this->InitGUI();

    this->DoFor(1);
}
//_____________________________________________
iCalibPIDphi::iCalibPIDphi(Int_t set)
{
    fSet = set;

    this->Init();

    // sum up all files contained in this runset
    iFileManager f(set, ECALIB_PID_E1);

    //hCBPhiVsPID = (TH2F*) h1;
    if (hCBPhiVsPID)
    {
        hCBPhiVsPID->Delete();
        hCBPhiVsPID=0;
    }

    if (f.GetHistogram(strPIDHistoName.Data()))
    {
        hCBPhiVsPID = (TH2F*) f.GetHistogram(strPIDHistoName.Data());
    }
    else
    {
        printf("\n ERROR: main histogram does not exist!!! \n");
        gSystem->Exit(0);
    }
    this->InitGUI();

    this->DoFor(1);
}
//_____________________________________________
iCalibPIDphi::~iCalibPIDphi()
{
    delete c1;
    delete c2;
    delete h4gr;
}

//_____________________________________________
void iCalibPIDphi::Init()
{
    // Initialize all variables in this class
    // print help
    this->Help();

    // Initialize all variables in this class
    //this->ReadFile( strPIDCalibFile );

    // get histo name
    strPIDHistoName = *this->GetConfig("PID.HName");


    // get time gain for CB chatch TDC's
    strPIDTgain = atof(this->GetConfig("PID.Tgain")->Data());

    for (Int_t i = 0; i < iConfig::kMaxPID; i++)
    {
        fNewPhi[i] = fOldPhi[i] = 0; //
        mean_gaus[i] = 0;
        hProj[i]   = 0; //
        lOffset[i] = 0;

    }


    // iMySQLReader
    //Char_t szTableName[64];
    //sprintf( szTableName, "%s_pid_phi", strTarget.Data() );
    //this->ReadDBtable( fSet, fOldPhi, iConfig::kMaxPID, szTableName );

    return;
}
//_____________________________________________
void iCalibPIDphi::InitGUI()
{
    c1 = new TCanvas("c1", "", 0, 0, 400, 800);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hCBPhiVsPID->Draw("colz");

    // create graph
    c2 = new TCanvas("c2", "",
                     430, 0, 900, 400);
    h4gr = new TH2F("h4gr", ";PID Number;CB #Phi [degree]",
                    26, -1, 25, 360, -180, 180);
    h4gr->SetStats(kFALSE);
    h4gr->Draw();

    grNewPhi = new TGraphErrors();
    grNewPhi->SetNameTitle("gr_new_", "");
    grNewPhi->SetMarkerStyle(7);
    grNewPhi->SetLineColor(27);

    grOldPhi = new TGraphErrors();
    grOldPhi->SetNameTitle("gr_old_", "");
    grOldPhi->SetMarkerStyle(7);
    grOldPhi->SetMarkerColor(2);
    grOldPhi->SetLineColor(2);

    Char_t szName[32];
    for (Int_t i = 0; i < iConfig::kMaxPID; i++)
    {
        sprintf(szName, "fFitPeak_%i", i);
        fFitPeak[(i)] = new TF1(szName, "pol1 + gaus(2)");

        lOffset[i] = new TLine();

        grOldPhi->SetPoint(i, (i+1), fOldPhi[i]);
    }
    return;
}
//_____________________________________________
void iCalibPIDphi::Help()
{
    cout <<"****************************************************************" << endl;
    cout <<"* You are working with PID Phi calibration modul               *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"* using CB Phi tracks                                          *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"****************************************************************" << endl;
    return;
}

//_____________________________________________
void iCalibPIDphi::DrawThis(Int_t id)
{
    // draw all histograms on 2 canvas
    //
    c1->cd(2);
    if (hProj[(id-1)])
    {
        //       hProj[(id-1)]->GetXaxis()->SetRangeUser(0, 300);
        hProj[(id-1)]->Draw();
    }

    if (fFitPeak[(id-1)])
    {
        fFitPeak[(id-1)]->SetLineColor(2);
        fFitPeak[(id-1)]->Draw("same");

        lOffset[(id-1)]->Draw("same");
    }
    //   c1->Modified();
    c1->Update();

    //
    c2->cd();
    grNewPhi->Draw("p");
    grOldPhi->Draw("p");
    //   c2->Modified();
    c2->Update();

    return;
}

//_____________________________________________
void iCalibPIDphi::GetProjection(Int_t id)
{
    // get projection from 2D histogram
    //
    Char_t szName[32];
    sprintf(szName, "hProj_%i", id);
    //  hProj[(id-1)] = (TH1D*) hCBPhiVsPID->ProjectionY(szName, id, id, "e");
    //  hProj[(id-1)] = (TH1D*) hCBPhiVsPID->ProjectionY(szName, id, id);
    hProj[(id-1)] = (TH1D*) hCBPhiVsPID->ProjectionX(szName, id, id);

    return;
}

//_____________________________________________
void iCalibPIDphi::Calculate(Int_t id)
{
    // calculate new gain
    //
    if (hProj[(id-1)]->GetEntries())
    {
        if (lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)])
        {
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();
            cout << "Mean Gauss and Line not equal" << endl;
        }
        //
        //fNewPhi[(id-1)] = mean_gaus[(id-1)]-fOldPhi[(id-1)];
        fNewPhi[(id-1)] = mean_gaus[(id-1)];
        //fOldPhi[(id-1)] = PIDphi[(id-1)];

        printf("Element: %03i \t new = %8.4f \t old = %8.4f \n",
               id, fNewPhi[(id-1)], fOldPhi[(id-1)]);

        grNewPhi->SetPoint((id-1), id, mean_gaus[(id-1)]);
        grNewPhi->SetPointError((id-1), 0., fFitPeak[(id-1)]->GetParError(3));
    }
    else
    {
        printf("Element: %03i \t new = %f \t old = %f \n",
               id, 0.0, 0.0);
    }

    return;
}

//_____________________________________________
void iCalibPIDphi::DoFor(Int_t id)
{
    // go through all crystals
    //
    currentCrystal = id;
    this->GetProjection(id);

    if ((hProj[(id-1)])->GetEntries()) this->DoFit(id);
    //this->FitPIDphi( id, hProj[(id-1)] );

    this->DrawThis(id);

    return;
}
//------------------------------------------------------------------------------
void iCalibPIDphi::DoFit(Int_t id)
{
    // Fit the Pi0 peak of an element in the corresponding histogram

    //
    Int_t maxbin = hProj[(id-1)]->GetMaximumBin();
    peackval = hProj[(id-1)]->GetBinCenter(maxbin);

    cout << "Peakval: " << peackval << endl ;

    fFitPeak[(id-1)]->SetRange(peackval-50, peackval+50);

    //peak splitted
    if (((peakval + 50) > 180) || ((peackval-50) <180))
    {
        Int_t count = 1;
        for (Int_t binx = (hProj[(id-1)]->FindBin(-180)); binx < (hProj[(id-1)]->FindBin(180)+1); binx++)
        {
            if (hProj[(id-1)]->GetBinCenter(binx)<=0) hProj[(id-1)]->Fill(180 + count, (hProj[(id-1)]->GetBinContent(binx)));
            else hProj[(id-1)]->Fill(-3*180 + count, (hProj[(id-1)]->GetBinContent(binx)));
            count++;
        }
    }

    fFitPeak[(id-1)]->SetLineColor(2);

    fFitPeak[(id-1)]->SetParameters(0.0, 0.0,
                                    hProj[(id-1)]->GetMaximum(), peackval, 8.);
    hProj[(id-1)]->Fit(fFitPeak[(id-1)], "+R0Q");
    mean_gaus[(id-1)] = fFitPeak[(id-1)]->GetParameter(3); // store peak value
    erro_gaus[(id-1)] = fFitPeak[(id-1)]->GetParError(3);  // store peak error

    if (mean_gaus[(id-1)]>180)
    {
        Double_t mean = -360 + mean_gaus[(id-1)];
        mean_gaus[(id-1)] = mean;
    }
    if (mean_gaus[(id-1)]<-180)
    {
        Double_t mean = 360 - mean_gaus[(id-1)];
        mean_gaus[(id-1)] = mean;
    }
    //
    lOffset[(id-1)]->SetVertical();
    lOffset[(id-1)]->SetLineColor(4);
    lOffset[(id-1)]->SetLineWidth(3);
    lOffset[(id-1)]->SetY1(0);
    lOffset[(id-1)]->SetY2(hProj[(id-1)]->GetMaximum() + 20);
    lOffset[(id-1)]->SetX1(mean_gaus[(id-1)]);
    lOffset[(id-1)]->SetX2(mean_gaus[(id-1)]);

    //   // Do check if everithing is ok
    //   if( fPol1Gaus[(id-1)]->GetParameter(4) < 5. ||
    //       fPol1Gaus[(id-1)]->GetParameter(4) > 10. )

    return;
}
//_____________________________________________
void iCalibPIDphi::Write()
{
    //
    //  this->WriteConfigFile_PID( fNewPhi );

    //Write in DB
    //Char_t szTableName[64];
    //sprintf( szTableName, "%s_pid_phi", strTarget.Data() );
    //this->WriteInDB( fSet, fNewPhi, iConfig::kMaxPID, szTableName );

    for (Int_t i = 0; i < iConfig::kMaxPID; i++)
    {
        printf("element: %d       phi: %f\n", i, fNewPhi[i]);
    }

    //save pictures
    Char_t szName[100];
    sprintf(szName,
            "%s/pid/phi/hGr_set%02i.gif",
            this->GetConfig("HTML.PATH")->Data(),
            fSet);
    Char_t szName1[100];
    sprintf(szName1,
            "%s/pid/phi/hPID_set%02i.gif",
            this->GetConfig("HTML.PATH")->Data(),
            fSet);

    //check if Directory for pictures exist, otherwise create
    Char_t szMakeDir[100];
    sprintf(szMakeDir,
            "mkdir -p %s/pid/phi/",
            this->GetConfig("HTML.PATH")->Data());
    gSystem->Exec(szMakeDir);

    c1->SaveAs(szName1);
    c2->SaveAs(szName);
    return;
}

// EOF
