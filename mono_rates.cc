#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "DetectorResponse.h"
#include "FormFactor.h"
#include "NuFlux.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <math.h>
#include <map>

#include "xscns.h"

int main(int argc, char *argv[])
{

  if (argc < 2)
  {
    std::cout << "Usage:  ./sns_rates [jsonfile]" << std::endl;
    exit(0);
  }

  const char *jsonfile = argv[1];

  std::string jsonfilename = "jsonfiles/" + std::string(jsonfile) + ".json";

  // Read a JSON file with the parameters
  std::ifstream i(jsonfilename);
  json j;
  i >> j;

  // print values
  std::cout << j << '\n';

  // Made these before adding json functionality
#include "isomaps.h"
  // Info for relevant mixtures
#include "mixtures.h"

  double wnumu = 0;
  double wnumubar = 0;
  double wnue = 1.;
  double wnuebar = 0.0;
  double wnutau = 0.0;
  double wnutaubar = 0.0;

  std::cout << "Flavor weights: " << wnumu << " " << wnumubar << " " << wnue << std::endl;

  // Set up the form factor

  std::string ffname = j["formfactor"]["type"];

  int noff = 0.;
  if (ffname == "unity")
  {
    noff = 1.;
  }


  FormFactor **ffpv;
  ffpv = new FormFactor *[max_components];
  FormFactor **ffpa;
  ffpa = new FormFactor *[max_components];

  FormFactor **ffnv;
  ffnv = new FormFactor *[max_components];
  FormFactor **ffna;
  ffna = new FormFactor *[max_components];

  // Set up the flux

  Monochromatic* monoflux = new Monochromatic();
  monoflux->SetEnergy(j["flux"]["energy"]);
  monoflux->SetFlavor(-3);

  double kmax = monoflux->GetEnergy();

  // Set up the detector response-- this is an overall detector response

  DetectorResponse *detresp = new DetectorResponse();

  double recoilthresh = 0.; // MeVr
  double eethresh = 0.;     // MeVr
  double qcthresh = 0.;     // Collected charge

  double recoilupperthresh = 0.; // MeVr
  double eeupperthresh = 0.;     // MeVr
  double qcupperthresh = 0.;     // Collected charge

  std::string eff_type = j["detectorresponse"]["efftype"];
  detresp->SetEfficType(eff_type.c_str());

  std::string effname = j["detectorresponse"]["efficiencyfile"];

  std::cout << "efficiency filename: " << effname << std::endl;
  if (effname != "none")
  {

    std::string eff_filename;
    eff_filename = "eff/" + std::string(effname);
    detresp->SetEfficFilename(eff_filename.c_str());
    detresp->ReadEfficFile();
  }
  if (eff_type == "erecoil")
  {
    recoilthresh = j["detectorresponse"]["stepthresh"];
    detresp->SetStepThresh(recoilthresh); // Not actually needed
    recoilupperthresh = j["detectorresponse"]["upperthresh"];
    detresp->SetUpperThresh(recoilupperthresh);
  }
  else if (eff_type == "eee")
  {
    eethresh = j["detectorresponse"]["stepthresh"];
    detresp->SetStepThresh(eethresh);
    eeupperthresh = j["detectorresponse"]["upperthresh"];
    detresp->SetUpperThresh(eeupperthresh);
  }
  else if (eff_type == "qc")
  {
    qcthresh = j["detectorresponse"]["stepthresh"];
    detresp->SetStepThresh(qcthresh);
    qcupperthresh = j["detectorresponse"]["upperthresh"];
    detresp->SetUpperThresh(qcupperthresh);
  }

  // Set up the material
  std::string material = j["material"];

  std::ofstream outfile;
  std::string outfilename;
  //  outfilename = "sns_diff_rates-"+material+"-"+std::string(ffname)+".out";
  outfilename = "out/diff_rates-" + std::string(jsonfile) + "-" + material + "-" + ffname + ".out";
  outfile.open(outfilename);
  std::cout << outfilename << std::endl;

  double M;
  double Delta;
  int Nn, Z, A;
  int Zdiff, Ndiff;

  std::string matname = material;

  // These are defined in mixtures include
  std::vector<double> fraction = molar_fraction[material];
  std::vector<std::string> isotope_component = isotopes[material];

  std::cout << "Material " << matname << std::endl;

  // Quenching filename info

  std::string qftype = j["detectorresponse"]["qftype"];
  std::string qfname = j["detectorresponse"]["qfname"];

  double Ntargets = 0; // Total targets
  int is = 0;
  std::vector<std::string>::iterator v = isotope_component.begin();
  std::string isotope;

  // First get the total mass.  Get also the maximum recoil values

  // double erecmaxvals[max_components];
  double Mtot = 0;
  v = isotope_component.begin();

  DetectorResponse **qffunc;
  qffunc = new DetectorResponse *[max_components];

  // First loop over materials, to set up material-specific arrays
  double minM = 1.e10;
  while (v != isotope_component.end())
  {

    isotope = *v;
    std::cout << "isotope" << isotope << std::endl;
    std::string isoname = std::string(isotope);

    Z = Zs[std::string(isotope)];
    Nn = Ns[std::string(isotope)];
    Delta = Deltas[std::string(isotope)];
    M = (Z + Nn) * amu - Z * me + Delta;
    if (M < minM)
    {
      minM = M;
    }
    Mtot += M * fraction[is];
    // erecmaxvals[is] = 2*kmax*kmax/(M+2*kmax);

    // Set up the form factor for this isotope

    //       std::cout << "Mass "<<M<<std::endl;


    double nvrfact = j["formfactor"]["nvrfact"];
    double narfact = j["formfactor"]["narfact"];
    double pvrfact = j["formfactor"]["pvrfact"];
    double parfact = j["formfactor"]["parfact"];


    if (ffname == "helm")
    {

      double nvsfact = j["formfactor"]["nvsfact"];
      double nasfact = j["formfactor"]["nasfact"];
      double pvsfact = j["formfactor"]["pvsfact"];
      double pasfact = j["formfactor"]["pasfact"];

      std::cout<<"hi"<<std::endl;

      Helm *helmffnv = new Helm();
      ffnv[is] = helmffnv;
      helmffnv->Setsval(nvsfact);
      helmffnv->SetRfac(nvrfact);

      Helm *helmffna = new Helm();
      ffna[is] = helmffna;
      helmffna->Setsval(nasfact);
      helmffna->SetRfac(narfact);

      Helm *helmffpv = new Helm();
      ffpv[is] = helmffpv;
      helmffpv->Setsval(pvsfact);
      helmffpv->SetRfac(pvrfact);

      Helm *helmffpa = new Helm();
      ffpa[is] = helmffpa;
      helmffpa->Setsval(pasfact);
      helmffpa->SetRfac(parfact);
    }

    else if (ffname == "klein")
    {

      double nvak = j["formfactor"]["nvak"];
      double naak = j["formfactor"]["naak"];
      double pvak = j["formfactor"]["pvak"];
      double paak = j["formfactor"]["paak"];
      double nvskin = j["formfactor"]["nvskin"];
      double naskin = j["formfactor"]["naskin"];
      double pvskin = j["formfactor"]["pvskin"];
      double paskin = j["formfactor"]["paskin"];

      Klein *kleinffnv = new Klein();
      ffnv[is] = kleinffnv;
      kleinffnv->Setakval(nvak);
      kleinffnv->SetRfac(nvrfact);
      kleinffnv->Setskinfac(nvskin);

      Klein *kleinffna = new Klein();
      ffna[is] = kleinffna;
      kleinffna->Setakval(naak);
      kleinffna->SetRfac(narfact);
      kleinffna->Setskinfac(naskin);

      Klein *kleinffpv = new Klein();
      ffpv[is] = kleinffpv;
      kleinffpv->Setakval(pvak);
      kleinffpv->SetRfac(pvrfact);
      kleinffpv->Setskinfac(pvskin);

      Klein *kleinffpa = new Klein();
      ffpa[is] = kleinffpa;
      kleinffpa->Setakval(paak);
      kleinffpa->SetRfac(parfact);
      kleinffpv->Setskinfac(paskin);
    }
    else if (ffname == "horowitz")
    {
      Horowitz *horowitzffnv = new Horowitz();
      ffnv[is] = horowitzffnv;

      Horowitz *horowitzffna = new Horowitz();
      ffna[is] = horowitzffna;

      Horowitz *horowitzffpv = new Horowitz();
      ffpv[is] = horowitzffpv;

      Horowitz *horowitzffpa = new Horowitz();
      ffpa[is] = horowitzffpa;

      std::transform(isoname.begin(), isoname.end(), isoname.begin(), ::toupper);
      std::string horowitz_filename = isoname + ".FF";
      std::cout << horowitz_filename << std::endl;
      horowitzffnv->SetFFfilename(horowitz_filename.c_str());
      horowitzffnv->ReadFFfile();
      horowitzffnv->SetRfac(nvrfact);

      horowitzffna->SetFFfilename(horowitz_filename.c_str());
      horowitzffna->ReadFFfile();
      horowitzffna->SetRfac(narfact);

      //      std::cout << horowitz_filename << std::endl;
      // Not really appropriate for protons, but using the structure
      horowitzffpv->SetFFfilename(horowitz_filename.c_str());
      horowitzffpv->ReadFFfile();
      horowitzffpv->SetRfac(pvrfact);

      horowitzffpa->SetFFfilename(horowitz_filename.c_str());
      horowitzffpa->ReadFFfile();
      horowitzffpa->SetRfac(parfact);
    }

    A = Nn + Z;
    if (noff == 0)
    {
      ffnv[is]->SetA(A);
      ffna[is]->SetA(A);
      ffpv[is]->SetA(A);
      ffpa[is]->SetA(A);

      ffnv[is]->SetZ(Z);
      ffna[is]->SetZ(Z);
      ffpv[is]->SetZ(Z);
      ffpa[is]->SetZ(Z);
    }

    // Set up detector quenching factors for each component

    if (qftype == "poly")
    {
      std::string qffilename;
      qffilename = "qf/" + std::string(qfname) + "_" + isoname + "_qf.txt";
      DetectorResponse *qf = new DetectorResponse();

      std::cout << "Quenching factor: " << qffilename << std::endl;
      qffunc[is] = qf;
      qffunc[is]->SetQFPolyFilename(qffilename.c_str());
      qffunc[is]->ReadQFPolyFile();
    }
    else if (qftype == "numerical")
    {
      std::string qffilename;
      qffilename = "qf/" + std::string(qfname) + "_" + isoname + "_qfnum.txt";
      DetectorResponse *qf = new DetectorResponse();

      std::cout << "Quenching factor: " << qffilename << std::endl;
      qffunc[is] = qf;
      qffunc[is]->SetQFFilename(qffilename.c_str());
      qffunc[is]->ReadQFFile();
    }
    v++;
    is++;
  }
  // Overall norm factor

  double detector_mass = j["mass"]; // tons

  double hours = 1.0;
  double exposure = 3600. * hours;

  double norm_factor = 1.0; // detector_mass * exposure * usernorm;

  // Set up arrays for quenched total rates... could make this a stl vec
  // but this is probably more efficient

  // Use the mass of the lightest component
  double erecmaxall = 2 * kmax * kmax / (minM + 2 * kmax);
  std::cout << "erecmaxall " << erecmaxall << std::endl;

  // double recoilthresh = 0.013; //MeVr
  double erecstart = 0; // recoilthresh;
  double erecend = recoilupperthresh > recoilthresh ? std::min(erecmaxall, recoilupperthresh) : erecmaxall;

  // double erecstep = 0.0001;
  double erecstep = 0.00001;

  if (j.find("erecstep") != j.end())
  {
    erecstep = j["erecstep"];
  }

  // Set up the recoil energy arrays

  int maxiq;
  maxiq = int((erecend - erecstart) / erecstep) + 1;

  double *Er = new double[maxiq];
  double **Eee = new double *[max_components];
  double **dNdEee = new double *[max_components];
  double **dNdEr = new double *[max_components];
  for (int iee = 0; iee < max_components; iee++)
  {
    Eee[iee] = new double[maxiq];
    dNdEee[iee] = new double[maxiq];
    dNdEr[iee] = new double[maxiq];
  }

  double *dNdErall = new double[maxiq];

  // Now compute the differential recoil spectra

  double Erec;
  double knu;

  std::cout << "erecmaxall " << erecmaxall << std::endl;

    // The totals
  double toterecoil = 0.;
  double totevents = 0.;
  double toteventsnue = 0.;
  double toteventsnuebar = 0.;
  double toteventsnumu = 0.;
  double toteventsnumubar = 0.;
  double toteventsnutau = 0.;
  double toteventsnutaubar = 0.;

  int iq = 0;
  // Loop over recoil energy
  for (Erec = erecstart; Erec <= erecend; Erec += erecstep)
  {

    Er[iq] = Erec;
    std::cout << "Erec " << Erec << std::endl;

    // Contributions for each component
    double diffrate_e_vec[max_components] = {0.};
    double diffrate_ebar_vec[max_components] = {0.};
    double diffrate_mu_vec[max_components] = {0.};
    double diffrate_mubar_vec[max_components] = {0.};
    double diffrate_tau_vec[max_components] = {0.};
    double diffrate_taubar_vec[max_components] = {0.};

    double diffrate_e_axial[max_components] = {0.};
    double diffrate_ebar_axial[max_components] = {0.};
    double diffrate_mu_axial[max_components] = {0.};
    double diffrate_mubar_axial[max_components] = {0.};
    double diffrate_tau_axial[max_components] = {0.};
    double diffrate_taubar_axial[max_components] = {0.};

    double diffrate_e_interf[max_components] = {0.};
    double diffrate_ebar_interf[max_components] = {0.};
    double diffrate_mu_interf[max_components] = {0.};
    double diffrate_mubar_interf[max_components] = {0.};
    double diffrate_tau_interf[max_components] = {0.};
    double diffrate_taubar_interf[max_components] = {0.};

    double diffrate_e_mag[max_components] = {0.};
    double diffrate_ebar_mag[max_components] = {0.};
    double diffrate_mu_mag[max_components] = {0.};
    double diffrate_mubar_mag[max_components] = {0.};
    double diffrate_tau_mag[max_components] = {0.};
    double diffrate_taubar_mag[max_components] = {0.};

    // Sum for each component,  not quenched

    double sum_diffrate_e_vec = 0;
    double sum_diffrate_ebar_vec = 0;
    double sum_diffrate_mu_vec = 0;
    double sum_diffrate_mubar_vec = 0;
    double sum_diffrate_tau_vec = 0;
    double sum_diffrate_taubar_vec = 0;

    double sum_diffrate_e_axial = 0;
    double sum_diffrate_ebar_axial = 0;
    double sum_diffrate_mu_axial = 0;
    double sum_diffrate_mubar_axial = 0;
    double sum_diffrate_tau_axial = 0;
    double sum_diffrate_taubar_axial = 0;

    double sum_diffrate_e_interf = 0;
    double sum_diffrate_ebar_interf = 0;
    double sum_diffrate_mu_interf = 0;
    double sum_diffrate_mubar_interf = 0;
    double sum_diffrate_tau_interf = 0;
    double sum_diffrate_taubar_interf = 0;

    double sum_diffrate_e_mag = 0;
    double sum_diffrate_ebar_mag = 0;
    double sum_diffrate_mu_mag = 0;
    double sum_diffrate_mubar_mag = 0;
    double sum_diffrate_tau_mag = 0;
    double sum_diffrate_taubar_mag = 0;

    // With efficiency, which is a function of Erec in MeV in this formuation

    double recoil_eff_factor = 1;
    if (effname != "none" && eff_type == "erecoil")
    {
      recoil_eff_factor = detresp->efficnum(Erec);
    }

    //     double eff_factor = 1.;
    //     std::cout << "recoil eff factor "<<recoil_eff_factor<<std::endl;
    // Skip if too small contribution
    if (recoil_eff_factor > 0)
    {

      v = isotope_component.begin();
      // Now loop over components
      is = 0;
      while (v != isotope_component.end())
      {

        isotope = *v;
        //	  std::cout << "isotope"<< isotope << std::endl;

        Z = Zs[std::string(isotope)];
        Nn = Ns[std::string(isotope)];
        Delta = Deltas[std::string(isotope)];
        M = (Z + Nn) * amu - Z * me + Delta;

        Zdiff = Zdiffs[std::string(isotope)];
        Ndiff = Ndiffs[std::string(isotope)];

        mass_fraction[is] = M / Mtot * fraction[is];

        A = Nn + Z;
        //	 std::cout << " Z "<<Z<<" N "<<Nn<<" A "<<A<<" M "<<M << " "<<mass_fraction[is]<<std::endl;

        // Loop over neutrino energy contributions

        // Minimum neutrino energy contributing to a given recoil energy

        double knumin = 0.5 * (Erec + sqrt(Erec * Erec + 2 * M * Erec));
        double hbarc = 197.327;                      // MeV-fm, convert for Q in MeV for ff
        double Q = sqrt(2 * M * Erec + Erec * Erec); // MeV
        //	 double Q = sqrt(2*M*Erec); // MeV

        double qq = Q / hbarc;
        //    double ff2 = helmff->FFval(qq);
        double ffnvval;
        double ffnaval;
        double ffpvval;
        double ffpaval;
        if (noff == 0)
        {
          ffnvval = ffnv[is]->FFval(qq);
          ffnaval = ffna[is]->FFval(qq);
          ffpvval = ffpv[is]->FFval(qq);
          ffpaval = ffpa[is]->FFval(qq);
        }
        else
        {
          ffnvval = 1.;
          ffnaval = 1.;
          ffpvval = 1.;
          ffpaval = 1.;
        }


        double gv[2], ga[2], gabar[2];
        int pdgyr = j["couplings"]["pdgyear"];
        if (pdgyr == 0)
        {
          // Custom couplings, must be present in json file
          gv[0] = j["couplings"]["gvp"];
          gv[1] = j["couplings"]["gvn"];
          ga[0] = j["couplings"]["gap"];
          ga[1] = j["couplings"]["gan"];
          gabar[0] = ga[0] * -1;
          gabar[1] = ga[1] * -1;
        }
        else
        {
          sm_vector_couplings(pdgyr, gv);
          sm_axial_couplings(pdgyr, 1, ga);
          sm_axial_couplings(pdgyr, -1, gabar);
        }

        // Bundle the form factor contributions with the SM couplings, separately for p and n
        double GV_sm_wff = Z * gv[0] * ffpvval + Nn * gv[1] * ffnvval;
        double GA_sm_wff = Zdiff * ga[0] * ffpaval + Ndiff * ga[1] * ffnaval;
        double GA_sm_bar_wff = Zdiff * gabar[0] * ffpaval + Ndiff * gabar[1] * ffnaval;

        // Charge radius correction
        double mufact = 1.;

        if (j["couplings"]["chargeradiusfactor"] == "sehgal")
        {
          mufact = mufactor(Q);
        }

        double GV_sm_wff_e = GV_sm_wff;
        double GV_sm_wff_ebar = GV_sm_wff;
        double GV_sm_wff_mu = GV_sm_wff;
        double GV_sm_wff_mubar = GV_sm_wff;
        double GV_sm_wff_tau = GV_sm_wff;
        double GV_sm_wff_taubar = GV_sm_wff;

        double chgradcorr_e = 0.;
        double chgradcorr_ebar = 0.;
        double chgradcorr_mu = 0.;
        double chgradcorr_mubar = 0.;
        double chgradcorr_tau = 0.;
        double chgradcorr_taubar = 0.;

        int chgcorrtype = 0;

        if (j["couplings"]["chargeradiusfactor"] == "erler")
        {
          chgcorrtype = 1;
        }

        if (j["couplings"]["chargeradiusfactor"] == "giunti")
        {
          // Corrected
          chgcorrtype = 2;
        }

        if (j["couplings"]["chargeradiusfactor"] == "giuntiold")
        {
          // legacy
          chgcorrtype = 3;
        }

        if (chgcorrtype > 0 && chgcorrtype <= 3)
        {
          chgradcorr_e = chgradcorr(1, chgcorrtype);
          chgradcorr_ebar = chgradcorr(-1, chgcorrtype);
          chgradcorr_mu = chgradcorr(2, chgcorrtype);
          chgradcorr_mubar = chgradcorr(-2, chgcorrtype);
          chgradcorr_tau = chgradcorr(3, chgcorrtype);
          chgradcorr_taubar = chgradcorr(-3, chgcorrtype);
        }
        if (j["couplings"]["chargeradiusfactor"] == "tomalak")
        {
          // These are Q dependent
          // Note these should come with custom couplings
          chgradcorr_e = chgradcorr_tomalak(Q, 1);
          chgradcorr_ebar = chgradcorr_tomalak(Q, 1);
          chgradcorr_mu = chgradcorr_tomalak(Q, 2);
          chgradcorr_mubar = chgradcorr_tomalak(Q, 2);
          chgradcorr_tau = chgradcorr_tomalak(Q, 3);
          chgradcorr_taubar = chgradcorr_tomalak(Q, 3);
        }

        GV_sm_wff_e = Z * (gv[0] + chgradcorr_e) * ffpvval + Nn * gv[1] * ffnvval;
        GV_sm_wff_ebar = Z * (gv[0] + chgradcorr_ebar) * ffpvval + Nn * gv[1] * ffnvval;
        GV_sm_wff_mu = Z * (gv[0] + chgradcorr_mu) * ffpvval + Nn * gv[1] * ffnvval;
        GV_sm_wff_mubar = Z * (gv[0] + chgradcorr_mubar) * ffpvval + Nn * gv[1] * ffnvval;
        GV_sm_wff_tau = Z * (gv[0] + chgradcorr_tau) * ffpvval + Nn * gv[1] * ffnvval;
        GV_sm_wff_taubar = Z * (gv[0] + chgradcorr_taubar) * ffpvval + Nn * gv[1] * ffnvval;

        // Default is SM
        double GV_wff_e = GV_sm_wff_e;
        double GV_wff_ebar = GV_sm_wff_ebar;
        double GV_wff_mu = GV_sm_wff_mu;
        double GV_wff_mubar = GV_sm_wff_mubar;
        double GV_wff_tau = GV_sm_wff_tau;
        double GV_wff_taubar = GV_sm_wff_taubar;

        // Axial couplings are SM
        double GA_wff = GA_sm_wff;
        double GA_bar_wff = GA_sm_bar_wff;

        // Magnetic moment..by flavor not quite right but OK for now

        double munu_e = 0.;
        double munu_ebar = 0.;
        double munu_mu = 0.;
        double munu_mubar = 0.;
        double munu_tau = 0.;
        double munu_taubar = 0.;

        if (j.find("magmom") != j.end())
        {

          munu_e = j["magmom"]["nue"];
          munu_ebar = j["magmom"]["nuebar"];
          munu_mu = j["magmom"]["numu"];
          munu_mubar = j["magmom"]["numubar"];
          munu_tau = j["magmom"]["nutau"];
          munu_taubar = j["magmom"]["nutaubar"];
          munu_e *= 1.e-10;
          munu_ebar *= 1.e-10;
          munu_mu *= 1.e-10;
          munu_mubar *= 1.e-10;
          munu_tau *= 1.e-10;
          munu_taubar *= 1.e-10;
        }

        // Targets for one ton of material
        // Will be weighted by mass fraction

        double Nt = 1.e6 / (M / amu) * 6.0221409e23;

        //	 std::cout << "Number of targets "<<Nt<<std::endl;
        // A2: G^2/(2Pi) * hbarcinmeters^-4
        double ntfac = 1.0; //Nt;

        // Quenching factor for this component and Eee for this Erec

        double qfderiv = 1;
        if (qftype == "poly")
        {
          std::cout << "Erec: " << Erec << std::endl;
          Eee[is][iq] = qffunc[is]->qfpoly(Erec) * Erec;
          qfderiv = abs(qffunc[is]->qfpolyderiv(Erec));
        }
        else if (qftype == "numerical")
        {
          Eee[is][iq] = qffunc[is]->qfnum(Erec) * Erec;
          qfderiv = abs(qffunc[is]->qfnumderiv(Erec));
        }

        double drate_e_vec = 0;
        double drate_ebar_vec = 0;
        double drate_mu_vec = 0;
        double drate_mubar_vec = 0;
        double drate_tau_vec = 0;
        double drate_taubar_vec = 0;

        double drate_e_axial = 0;
        double drate_ebar_axial = 0;
        double drate_mu_axial = 0;
        double drate_mubar_axial = 0;
        double drate_tau_axial = 0;
        double drate_taubar_axial = 0;

        double drate_e_interf = 0;
        double drate_ebar_interf = 0;
        double drate_mu_interf = 0;
        double drate_mubar_interf = 0;
        double drate_tau_interf = 0;
        double drate_taubar_interf = 0;

        double drate_e_mag = 0;
        double drate_ebar_mag = 0;
        double drate_mu_mag = 0;
        double drate_mubar_mag = 0;
        double drate_tau_mag = 0;
        double drate_taubar_mag = 0;

        double knu = kmax;
        if (knu > knumin){

          drate_e_vec += diffxscnvec(knu, M, Erec);
          drate_ebar_vec += diffxscnvec(knu, M, Erec) ;
          drate_mu_vec += diffxscnvec(knu, M, Erec);
          drate_mubar_vec += diffxscnvec(knu, M, Erec) ;
          drate_tau_vec += diffxscnvec(knu, M, Erec);
          drate_taubar_vec += diffxscnvec(knu, M, Erec) ;

          drate_e_axial += diffxscnaxial(knu, M, Erec);
          drate_ebar_axial += diffxscnaxial(knu, M, Erec) ;
          drate_mu_axial += diffxscnaxial(knu, M, Erec);
          drate_mubar_axial += diffxscnaxial(knu, M, Erec) ;
          drate_tau_axial += diffxscnaxial(knu, M, Erec);
          drate_taubar_axial += diffxscnaxial(knu, M, Erec) ;

          drate_e_interf += diffxscninterf(knu, M, Erec);
          drate_ebar_interf += diffxscninterf(knu, M, Erec) ;
          drate_mu_interf += diffxscninterf(knu, M, Erec);
          drate_mubar_interf += diffxscninterf(knu, M, Erec) ;
          drate_tau_interf += diffxscninterf(knu, M, Erec);
          drate_taubar_interf += diffxscninterf(knu, M, Erec) ;

          drate_e_mag += 0; // diffxscnmag(knu, Erec);
          drate_ebar_mag += 0; // diffxscnmag(knu, Erec) ;
          drate_mu_mag += 0; // diffxscnmag(knu, Erec);
          drate_mubar_mag += 0; // diffxscnmag(knu, Erec) ;
          drate_tau_mag += 0; // diffxscnmag(knu, Erec);
          drate_taubar_mag += 0; // diffxscnmag(knu, Erec) ;

        }

          //  std::cout<<"drat_e_vec: "<<drate_e_vec<<std::endl;



        // Now multiply by target-dependent factors and add up this recoil energy bin

        std::cout << "drate_e_vec: " << drate_e_vec << std::endl;
        std::cout << "ntfac: " << ntfac << std::endl;
        std::cout << "GV_wff_e: " << GV_wff_e << std::endl;
        std::cout << "mass_fraction[is]: " << mass_fraction[is] << std::endl;

        diffrate_e_vec[is] += ntfac * pow(GV_wff_e, 2) * mass_fraction[is] * drate_e_vec * wnue;
        diffrate_ebar_vec[is] += ntfac * pow(GV_wff_ebar, 2) * mass_fraction[is] * drate_ebar_vec * wnuebar;
        diffrate_mu_vec[is] += ntfac * pow(GV_wff_mu, 2) * mass_fraction[is] * drate_mu_vec * mufact * wnumu;
        diffrate_mubar_vec[is] += ntfac * pow(GV_wff_mubar, 2) * mass_fraction[is] * drate_mubar_vec * mufact * wnumubar;
        diffrate_tau_vec[is] += ntfac * pow(GV_wff_tau, 2) * mass_fraction[is] * drate_tau_vec * wnutau;
        diffrate_taubar_vec[is] += ntfac * pow(GV_wff_taubar, 2) * mass_fraction[is] * drate_taubar_vec * wnutaubar;

        diffrate_e_axial[is] += ntfac * pow(GA_wff, 2) * mass_fraction[is] * drate_e_axial * wnue;
        diffrate_ebar_axial[is] += ntfac * pow(GA_bar_wff, 2) * mass_fraction[is] * drate_ebar_axial * wnuebar;
        diffrate_mu_axial[is] += ntfac * pow(GA_wff, 2) * mass_fraction[is] * drate_mu_axial * mufact * wnumu;
        diffrate_mubar_axial[is] += ntfac * pow(GA_bar_wff, 2) * mass_fraction[is] * drate_mubar_axial * mufact * wnumubar;
        diffrate_tau_axial[is] += ntfac * pow(GA_wff, 2) * mass_fraction[is] * drate_tau_axial * wnutau;
        diffrate_taubar_axial[is] += ntfac * pow(GA_bar_wff, 2) * mass_fraction[is] * drate_taubar_axial * wnutaubar;

        diffrate_e_interf[is] += ntfac * GV_wff_e * GA_wff * mass_fraction[is] * drate_e_interf * wnue;
        diffrate_ebar_interf[is] += ntfac * GV_wff_ebar * GA_bar_wff * mass_fraction[is] * drate_ebar_interf * wnuebar;
        diffrate_mu_interf[is] += ntfac * GV_wff_mu * GA_wff * mass_fraction[is] * drate_mu_interf * mufact * wnumu;
        diffrate_mubar_interf[is] += ntfac * GV_wff_mubar * GA_bar_wff * mass_fraction[is] * drate_mubar_interf * mufact * wnumubar;
        diffrate_tau_interf[is] += ntfac * GV_wff_tau * GA_wff * mass_fraction[is] * drate_tau_interf * wnutau;
        diffrate_taubar_interf[is] += ntfac * GV_wff_taubar * GA_bar_wff * mass_fraction[is] * drate_taubar_interf * wnutaubar;

        diffrate_e_mag[is] += ntfac * pow(munu_e, 2) * pow(Z, 2) * mass_fraction[is] * drate_e_mag * wnue;
        diffrate_ebar_mag[is] += ntfac * pow(munu_ebar, 2) * pow(Z, 2) * mass_fraction[is] * drate_ebar_mag * wnuebar;
        diffrate_mu_mag[is] += ntfac * pow(munu_mu, 2) * pow(Z, 2) * mass_fraction[is] * drate_mu_mag * wnumu;
        diffrate_mubar_mag[is] += ntfac * pow(munu_mubar, 2) * pow(Z, 2) * mass_fraction[is] * drate_mubar_mag * wnumubar;
        diffrate_tau_mag[is] += ntfac * pow(munu_tau, 2) * pow(Z, 2) * mass_fraction[is] * drate_tau_mag * wnutau;
        diffrate_taubar_mag[is] += ntfac * pow(munu_taubar, 2) * pow(Z, 2) * mass_fraction[is] * drate_taubar_mag * wnutaubar;

        // Now add the contribution from this isotope to the sum

        sum_diffrate_e_vec += diffrate_e_vec[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_ebar_vec += diffrate_ebar_vec[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mu_vec += diffrate_mu_vec[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mubar_vec += diffrate_mubar_vec[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_tau_vec += diffrate_tau_vec[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_taubar_vec += diffrate_taubar_vec[is] * norm_factor * recoil_eff_factor;

        sum_diffrate_e_axial += diffrate_e_axial[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_ebar_axial += diffrate_ebar_axial[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mu_axial += diffrate_mu_axial[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mubar_axial += diffrate_mubar_axial[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_tau_axial += diffrate_tau_axial[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_taubar_axial += diffrate_taubar_axial[is] * norm_factor * recoil_eff_factor;

        sum_diffrate_e_interf += diffrate_e_interf[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_ebar_interf += diffrate_ebar_interf[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mu_interf += diffrate_mu_interf[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mubar_interf += diffrate_mubar_interf[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_tau_interf += diffrate_tau_interf[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_taubar_interf += diffrate_taubar_interf[is] * norm_factor * recoil_eff_factor;

        sum_diffrate_e_mag += diffrate_e_mag[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_ebar_mag += diffrate_ebar_mag[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mu_mag += diffrate_mu_mag[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_mubar_mag += diffrate_mubar_mag[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_tau_mag += diffrate_tau_mag[is] * norm_factor * recoil_eff_factor;
        sum_diffrate_taubar_mag += diffrate_taubar_mag[is] * norm_factor * recoil_eff_factor;

        // Sum for this Erec and isotope
        double sum_events_iso = 0;
        sum_events_iso = diffrate_e_vec[is] + diffrate_ebar_vec[is] + diffrate_mu_vec[is] + diffrate_mubar_vec[is] + diffrate_tau_vec[is] + diffrate_taubar_vec[is];
        sum_events_iso += diffrate_e_axial[is] + diffrate_ebar_axial[is] + diffrate_mu_axial[is] + diffrate_mubar_axial[is] + diffrate_tau_axial[is] + diffrate_taubar_axial[is];

        sum_events_iso += diffrate_e_interf[is] + diffrate_ebar_interf[is] + diffrate_mu_interf[is] + diffrate_mubar_interf[is] + diffrate_tau_interf[is] + diffrate_taubar_interf[is];

        sum_events_iso += diffrate_e_mag[is] + diffrate_ebar_mag[is] + diffrate_mu_mag[is] + diffrate_mubar_mag[is] + diffrate_tau_mag[is] + diffrate_taubar_mag[is];

        sum_events_iso *= norm_factor * recoil_eff_factor;

        // Now apply the quenching for this Ee and isotope component
        // sum_events_iso is dNderec

        dNdEr[is][iq] = sum_events_iso;
        dNdErall[iq] += sum_events_iso;

        if (qfderiv > 0)
        {
          dNdEee[is][iq] = sum_events_iso / qfderiv;
        }
        else
        {
          dNdEee[is][iq] = 0.;
        }

        std::cout << is<<" "<<"Erec "<<Erec<<" mass frac "<<mass_fraction[is]<<" "<<std::endl;
        std::cout << "  norm_factor "<<norm_factor<<" recoil_eff_factor "<<recoil_eff_factor<<std::endl;
        std::cout << "  diffrate_e_vec[is] "<<diffrate_e_vec[is]<<std::endl;
        std::cout<<"  dNdErall[iq] "<<dNdErall[iq]<<std::endl;

        //	 cout << "is, rate "<<is<<" "<<diffrate_e_vec[is]<<endl;

        v++;
        is++;

      } // End of loop over material components

    } // End of efficiency factor check

    //     std::cout <<Erec<<scientific<<" "<<sum_diffrate_e_vec<<" "<<sum_diffrate_ebar_vec<<" "<<sum_diffrate_mu_vec<<" "<<sum_diffrate_mubar_vec<<" "<<sum_diffrate_tau_vec<<" "<<sum_diffrate_taubar_vec<<" "<<sum_diffrate_e_axial<<" "<<sum_diffrate_ebar_axial<<" "<<sum_diffrate_mu_axial<<" "<<sum_diffrate_mubar_axial<<" "<<sum_diffrate_tau_axial<<" "<<sum_diffrate_taubar_axial<<" "<<sum_diffrate_e_interf<<" "<<sum_diffrate_ebar_interf<<" "<<sum_diffrate_mu_interf<<" "<<sum_diffrate_mubar_interf<<" "<<sum_diffrate_tau_interf<<" "<<sum_diffrate_taubar_interf <<std::endl;

    // Only want diff values in scientific format
    std::cout.unsetf(ios::fixed | ios::scientific);

         sum_diffrate_e_vec *= norm_factor*wnue;
         sum_diffrate_ebar_vec *= norm_factor;
         sum_diffrate_mu_vec *= norm_factor*wnumu;
         sum_diffrate_mubar_vec *= norm_factor*wnumubar;
         sum_diffrate_tau_vec *= norm_factor;
         sum_diffrate_taubar_vec *= norm_factor;

         sum_diffrate_e_axial *= norm_factor*wnue;
         sum_diffrate_ebar_axial *= norm_factor;
         sum_diffrate_mu_axial *= norm_factor*wnumu;
         sum_diffrate_mubar_axial *= norm_factor*wnumubar;
         sum_diffrate_tau_axial *= norm_factor;
         sum_diffrate_taubar_axial *= norm_factor;

         sum_diffrate_e_interf *= norm_factor*wnue;
         sum_diffrate_ebar_interf *= norm_factor;
         sum_diffrate_mu_interf *= norm_factor*wnumu;
         sum_diffrate_mubar_interf *= norm_factor*wnumubar;
         sum_diffrate_tau_interf *= norm_factor;
         sum_diffrate_taubar_interf *= norm_factor;

    outfile << Erec << scientific << " "  << sum_diffrate_e_vec << " " << sum_diffrate_ebar_vec << " " 
                                          << sum_diffrate_mu_vec << " " << sum_diffrate_mubar_vec << " " 
                                          << sum_diffrate_tau_vec << " " << sum_diffrate_taubar_vec << " " 
                                          << sum_diffrate_e_axial << " " << sum_diffrate_ebar_axial << " " 
                                          << sum_diffrate_mu_axial << " " << sum_diffrate_mubar_axial << " " 
                                          << sum_diffrate_tau_axial << " " << sum_diffrate_taubar_axial << " " 
                                          << sum_diffrate_e_interf << " " << sum_diffrate_ebar_interf << " " 
                                          << sum_diffrate_mu_interf << " " << sum_diffrate_mubar_interf << " " 
                                          << sum_diffrate_tau_interf << " " << sum_diffrate_taubar_interf << " " 
                                          << sum_diffrate_e_mag << " " << sum_diffrate_ebar_mag << " " 
                                          << sum_diffrate_mu_mag << " " << sum_diffrate_mubar_mag << " " 
                                          << sum_diffrate_tau_mag << " " << sum_diffrate_taubar_mag << std::endl;
    // Reset the format
    std::cout.unsetf(ios::fixed | ios::scientific);

    double events = 0;
    events = sum_diffrate_e_vec + sum_diffrate_ebar_vec + sum_diffrate_mu_vec + sum_diffrate_mubar_vec + sum_diffrate_tau_vec + sum_diffrate_taubar_vec;
    events += sum_diffrate_e_axial + sum_diffrate_ebar_axial + sum_diffrate_mu_axial + sum_diffrate_mubar_axial + sum_diffrate_tau_axial + sum_diffrate_taubar_axial;
    events += sum_diffrate_e_interf + sum_diffrate_ebar_interf + sum_diffrate_mu_interf + sum_diffrate_mubar_interf + sum_diffrate_tau_interf + sum_diffrate_taubar_interf;

    events += sum_diffrate_e_mag + sum_diffrate_ebar_mag + sum_diffrate_mu_mag + sum_diffrate_mubar_mag + sum_diffrate_tau_mag + sum_diffrate_taubar_mag;

    toteventsnue += erecstep * (sum_diffrate_e_vec + sum_diffrate_e_axial + sum_diffrate_e_interf + sum_diffrate_e_mag);

    toteventsnumu += erecstep * (sum_diffrate_mu_vec + sum_diffrate_mu_axial + sum_diffrate_mu_interf + sum_diffrate_mu_mag);

    toteventsnumubar += erecstep * (sum_diffrate_mubar_vec + sum_diffrate_mubar_axial + sum_diffrate_mubar_interf + sum_diffrate_mubar_mag);

    totevents += events * erecstep;

    toterecoil += events * Erec * erecstep;

    // Increment bin for quenching

    iq++;

  } // End of loop over Erec

  if (recoilupperthresh > recoilthresh)
  {
    std::cout << "Total events over " << recoilthresh * 1000. << " keVr and under " << recoilupperthresh * 1000 << " keVr: " << totevents << std::endl;
  }
  else
  {
    std::cout << "Total events over " << recoilthresh * 1000. << " keVr: " << totevents << std::endl;
  }
  std::cout << "Total recoil energy deposited:  " << toterecoil << std::endl;

  outfile.close();

  std::ofstream integraloutfile;
  outfilename = "out/diff_rates-" + std::string(jsonfile) + "-" + material + "-" + ffname + "-integral.out";

  std::cout << outfilename << std::endl;
  integraloutfile.open(outfilename);

  integraloutfile << j << '\n';
  if (recoilupperthresh > recoilthresh)
  {
    integraloutfile << "Total events over " << recoilthresh * 1000. << " keVr and under " << recoilupperthresh * 1000 << " keVr: " << totevents << std::endl;
  }
  else
  {
    integraloutfile << "Total events over " << recoilthresh * 1000. << " keVr: " << totevents << std::endl;
  }
  integraloutfile.close();

  // Output by isotope, integrated over flavor.
  // Can also output quenched stuff here
  // Loop over isotopes

  v = isotope_component.begin();
  // Now loop over components
  is = 0;
  while (v != isotope_component.end())
  {

    isotope = *v;
    //	  std::cout << "isotope"<< isotope << std::endl;
    std::string isoname = std::string(isotope);

    std::ofstream isooutfile;
    outfilename = "out/diff_rates-" + std::string(jsonfile) + "-" + material + "-" + ffname + "-" + isoname + ".out";

    std::cout << outfilename << std::endl;
    isooutfile.open(outfilename);

    int ie;
    for (ie = 0; ie < iq; ie++)
    {

      isooutfile << Er[ie] << "  " << dNdEr[is][ie] << endl;
    }
    isooutfile.close();
    v++;
    is++;
  }

  // Integrated over flavor and isotope

  std::ofstream allisooutfile;
  outfilename = "out/diff_rates_alliso-" + std::string(jsonfile) + "-" + material + "-" + ffname + ".out";

  std::cout << outfilename << std::endl;
  allisooutfile.open(outfilename);

  int ie;
  for (ie = 0; ie < iq; ie++)
  {

    allisooutfile << Er[ie] << "  " << dNdErall[ie] << endl;
  }
  allisooutfile.close();


  // Total flux-averaged xscn

  Ntargets = 1.0; // 1.e6 / (Mtot / amu) * 6.0221409e23 * detector_mass;
  double totalnus = 1.0; // 1. * (wnue + wnumu + wnumubar) / (4 * M_PI * 1. * 1.) * exposure;
  double totalnue = 1.0; // 1. * wnue / (4 * M_PI * 1. * 1.) * exposure;
  double totalnumu = 1.0; // 1. * wnumu / (4 * M_PI * 1. * 1.) * exposure;
  double totalnumubar = 1.0; // 1. * wnumubar / (4 * M_PI * 1. * 1.) * exposure;

  std::cout<<"Total number of targets: "<<Ntargets<<std::endl;
  std::cout<<"Total number of neutrinos: "<<totalnus<<std::endl;

  std::cout << "Total flux-averaged cross section: " << totevents / Ntargets / totalnus * 1e38 << " x 10-38 cm^2" << std::endl;
  std::cout << "Total flux-averaged cross section, nue: " << toteventsnue / Ntargets / totalnue * 1e38 << " x 10-38 cm^2" << std::endl;
  std::cout << "Total flux-averaged cross section, numu: " << toteventsnumu / Ntargets / totalnumu * 1e38 << " x 10-38 cm^2" << std::endl;
  std::cout << "Total flux-averaged cross section, numubar: " << toteventsnumubar / Ntargets / totalnumubar * 1e38 << " x 10-38 cm^2" << std::endl;
  std::cout << "Total flux-averaged cross section, numu+numubar: " << (toteventsnumu + toteventsnumubar) / Ntargets / (totalnumu + totalnumubar) * 1e38 << " x 10-38 cm^2" << std::endl;

  delete[] Er;
  delete[] Eee;
  delete[] dNdEee;
  delete[] dNdEr;
  delete[] dNdErall;

  return 0;
}
