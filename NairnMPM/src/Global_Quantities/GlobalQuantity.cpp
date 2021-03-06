/********************************************************************************
    GlobalQuantity.cpp
    nairn-mpm-fea
    
    Created by John Nairn on Mon Jan 12 2004.
    Copyright (c) 2004 John A. Nairn, All rights reserved. 
	
	Adding Global Quantity
		1. Add tag in GlobalQuantity.hpp
		2. Respond to string in GlobalQuantity(char *,int) initializer
		3. Add calculations in AppendQuantity()
********************************************************************************/

#include "stdafx.h"
#include "Global_Quantities/GlobalQuantity.hpp"
#include "NairnMPM_Class/NairnMPM.hpp"
#include "Materials/MaterialBase.hpp"
#include "Global_Quantities/ThermalRamp.hpp"
#include "Global_Quantities/BodyForce.hpp"
#include "MPM_Classes/MPMBase.hpp"
#include "Nodes/NodalPoint.hpp"
#include "System/ArchiveData.hpp"
#include "Boundary_Conditions/NodalVelBC.hpp"
#include "Boundary_Conditions/NodalTempBC.hpp"
#include "System/UnitsController.hpp"

// Single global contact law object
GlobalQuantity *firstGlobal=NULL;
static GlobalQuantity *lastGlobal=NULL;
static int numGlobal=0;
static char quote='"';

/*******************************************************************
	GlobalQuantity: Constructors and Destructors
*******************************************************************/

// Constructors
GlobalQuantity::GlobalQuantity()
{
}

// throws std::bad_alloc
GlobalQuantity::GlobalQuantity(char *quant,int whichOne)
{
	char nameStr[200];
	whichMat=whichOne;
	
	quantity = DecodeGlobalQuantity(quant,&subcode);
	
	// set name
	if(whichMat!=0)
		sprintf(nameStr,"%s mat %d",quant,whichMat);
	else
		strcpy(nameStr,quant);
	name=new char[strlen(nameStr)+1];
	strcpy(name,nameStr);
	
	// set color ID
	colorID=numGlobal % 10;
	
	// this object is currently the last one
	SetNextGlobal(NULL);
	
	// adjust previous global quantity or set firstGlobal if this is the first one
	if(lastGlobal!=NULL)
		lastGlobal->SetNextGlobal(this);
	else
		firstGlobal=this;
	lastGlobal=this;
	
	// count the number of objects
	numGlobal++;
}

// decode quant it to quantity ID and subcode (used for history variables)
int GlobalQuantity::DecodeGlobalQuantity(char *quant,int *hcode)
{
	int theQuant;
	
	// set quantity and subcode
	*hcode = 0;
	if(strcmp(quant,"sxx")==0 || strcmp(quant,"sRR")==0)
		theQuant=AVG_SXX;
	else if(strcmp(quant,"syy")==0 || strcmp(quant,"sZZ")==0)
		theQuant=AVG_SYY;
	else if(strcmp(quant,"sxy")==0 || strcmp(quant,"sRZ")==0)
		theQuant=AVG_SXY;
	else if(strcmp(quant,"szz")==0 || strcmp(quant,"sTT")==0)
		theQuant=AVG_SZZ;
	else if(strcmp(quant,"sxz")==0)
		theQuant=AVG_SXZ;
	else if(strcmp(quant,"syz")==0)
		theQuant=AVG_SYZ;
	
	else if(strcmp(quant,"exx")==0 || strcmp(quant,"eRR")==0)
		theQuant=AVG_EXX;
	else if(strcmp(quant,"eyy")==0 || strcmp(quant,"eZZ")==0)
		theQuant=AVG_EYY;
	else if(strcmp(quant,"exy")==0 || strcmp(quant,"eRZ")==0)
		theQuant=AVG_EXY;
	else if(strcmp(quant,"ezz")==0 || strcmp(quant,"eTT")==0)
		theQuant=AVG_EZZ;
	else if(strcmp(quant,"exz")==0)
		theQuant=AVG_EXZ;
	else if(strcmp(quant,"eyz")==0)
		theQuant=AVG_EYZ;
	
	else if(strcmp(quant,"Fxx")==0 || strcmp(quant,"FRR")==0)
		theQuant=AVG_FXX;
	else if(strcmp(quant,"Fxy")==0 || strcmp(quant,"FRZ")==0)
		theQuant=AVG_FXY;
	else if(strcmp(quant,"Fxz")==0)
		theQuant=AVG_FXZ;
	else if(strcmp(quant,"Fyx")==0 || strcmp(quant,"FZR")==0)
		theQuant=AVG_FYX;
	else if(strcmp(quant,"Fyy")==0 || strcmp(quant,"FZZ")==0)
		theQuant=AVG_FYY;
	else if(strcmp(quant,"Fyz")==0)
		theQuant=AVG_FYZ;
	else if(strcmp(quant,"Fzx")==0)
		theQuant=AVG_FZX;
	else if(strcmp(quant,"Fzy")==0)
		theQuant=AVG_FZY;
	else if(strcmp(quant,"Fzz")==0 || strcmp(quant,"FTT")==0)
		theQuant=AVG_FZZ;
	
	else if(strcmp(quant,"exxe")==0 || strcmp(quant,"eRRe")==0)
		theQuant=AVG_EXXE;
	else if(strcmp(quant,"eyye")==0 || strcmp(quant,"eZZe")==0)
		theQuant=AVG_EYYE;
	else if(strcmp(quant,"exye")==0 || strcmp(quant,"eRZe")==0)
		theQuant=AVG_EXYE;
	else if(strcmp(quant,"ezze")==0 || strcmp(quant,"eTTe")==0)
		theQuant=AVG_EZZE;
	else if(strcmp(quant,"exze")==0)
		theQuant=AVG_EXZE;
	else if(strcmp(quant,"eyze")==0)
		theQuant=AVG_EYZE;
	
	else if(strcmp(quant,"exxp")==0 || strcmp(quant,"eRRp")==0)
		theQuant=AVG_EXXP;
	else if(strcmp(quant,"eyyp")==0 || strcmp(quant,"eZZp")==0)
		theQuant=AVG_EYYP;
	else if(strcmp(quant,"exyp")==0 || strcmp(quant,"eRZp")==0)
		theQuant=AVG_EXYP;
	else if(strcmp(quant,"ezzp")==0 || strcmp(quant,"eTTp")==0)
		theQuant=AVG_EZZP;
	else if(strcmp(quant,"exzp")==0)
		theQuant=AVG_EXZP;
	else if(strcmp(quant,"eyzp")==0)
		theQuant=AVG_EYZP;
	
	else if(strcmp(quant,"Kinetic Energy")==0)
		theQuant=KINE_ENERGY;
	else if(strcmp(quant,"Grid Kinetic Energy")==0)
		theQuant=GRID_KINE_ENERGY;
	else if(strcmp(quant,"Strain Energy")==0)
		theQuant=STRAIN_ENERGY;
	else if(strcmp(quant,"Heat Energy")==0 || strcmp(quant,"Thermal Energy")==0)
		theQuant=HEAT_ENERGY;
	else if(strcmp(quant,"Entropy")==0)
		theQuant=ENTROPY_ENERGY;
	else if(strcmp(quant,"Internal Energy")==0)
		theQuant=INTERNAL_ENERGY;
	else if(strcmp(quant,"Helmholz Energy")==0)
		theQuant=HELMHOLZ_ENERGY;
	else if(strcmp(quant,"Interface Energy")==0)
		theQuant=INTERFACE_ENERGY;
	else if(strcmp(quant,"Friction Work")==0)
		theQuant=FRICTION_WORK;
	else if(strcmp(quant,"Work Energy")==0)
		theQuant=WORK_ENERGY;
	else if(strcmp(quant,"Plastic Energy")==0)
		theQuant=PLAS_ENERGY;
	
	else if(strcmp(quant,"velx")==0 || strcmp(quant,"velR")==0)
		theQuant=AVG_VELX;
	else if(strcmp(quant,"vely")==0 || strcmp(quant,"velZ")==0)
		theQuant=AVG_VELY;
	else if(strcmp(quant,"velz")==0)
		theQuant=AVG_VELZ;
	
	else if(strcmp(quant,"dispx")==0 || strcmp(quant,"dispR")==0)
		theQuant=AVG_DISPX;
	else if(strcmp(quant,"dispy")==0 || strcmp(quant,"dispZ")==0)
		theQuant=AVG_DISPY;
	else if(strcmp(quant,"dispz")==0)
		theQuant=AVG_DISPZ;
	
	else if(strcmp(quant,"temp")==0)
		theQuant=AVG_TEMP;
	else if(strcmp(quant,"concentration")==0)
		theQuant=WTFRACT_CONC;
	else if(strcmp(quant,"Step number")==0)
		theQuant=STEP_NUMBER;
	else if(strcmp(quant,"CPU time")==0)
		theQuant=CPU_TIME;
	else if(strcmp(quant,"Elapsed time")==0)
		theQuant=ELAPSED_TIME;
	else if(strcmp(quant,"alpha")==0)
		theQuant=GRID_ALPHA;
	else if(strcmp(quant,"palpha")==0)
		theQuant=PARTICLE_ALPHA;
	
	else if(strcmp(quant,"contactx")==0 || strcmp(quant,"contactR")==0)
		theQuant=TOT_FCONX;
	else if(strcmp(quant,"contacty")==0 || strcmp(quant,"contactZ")==0)
		theQuant=TOT_FCONY;
	else if(strcmp(quant,"contactz")==0)
		theQuant=TOT_FCONZ;
	
	else if(strcmp(quant,"reactionx")==0 || strcmp(quant,"reactionR")==0)
		theQuant=TOT_REACTX;
	else if(strcmp(quant,"reactiony")==0 || strcmp(quant,"reactionZ")==0)
		theQuant=TOT_REACTY;
	else if(strcmp(quant,"reactionz")==0)
		theQuant=TOT_REACTZ;
	
	else if(strcmp(quant,"px")==0 || strcmp(quant,"pR")==0)
		theQuant=LINMOMX;
	else if(strcmp(quant,"py")==0 || strcmp(quant,"pZ")==0)
		theQuant=LINMOMY;
	else if(strcmp(quant,"pZ")==0)
		theQuant=LINMOMZ;
	
	else if(strcmp(quant,"Lx")==0)
		theQuant=ANGMOMX;
	else if(strcmp(quant,"Ly")==0)
		theQuant=ANGMOMY;
	else if(strcmp(quant,"Lz")==0)
		theQuant=ANGMOMZ;
	
	else if(strcmp(quant,"heatWatts")==0)
		theQuant=TOT_REACTQ;
	
	else
	{	theQuant=UNKNOWN_QUANTITY;
		
		// possibly a history variable which must be "history n"
		if(strlen(quant)>7)
		{	char nameStr[200];
			strcpy(nameStr,quant);
			nameStr[7]=0;
			if(strcmp(nameStr,"history")==0)
			{	sscanf(quant,"%*s %d",hcode);
				theQuant=HISTORY_VARIABLE;
			}
		}
	}
	
	return theQuant;
}

#pragma mark GlobalQuantity: Methods

// append quantity
GlobalQuantity *GlobalQuantity::AppendName(char *fline)
{
	char nameStr[100];
	
	if(quantity==UNKNOWN_QUANTITY)
		return nextGlobal;
	
	sprintf(nameStr,"\t%c%s%c",quote,name,quote);
	strcat(fline,nameStr);
	return nextGlobal;
}

// append quantity
GlobalQuantity *GlobalQuantity::AppendQuantity(vector<double> &toArchive)
{
	int p;
	double value=0.,rho0,Vtot=0.,mp,Jp,Vp;
	int matid,qid=0;
	
	switch(quantity)
	{   // stresses (MPa in Legacy)
		case AVG_SZZ:
			qid=ZZ;
		case AVG_SXZ:
			if(quantity==AVG_SXZ) qid=XZ;
		case AVG_SYZ:
			if(quantity==AVG_SYZ) qid=YZ;
	    case AVG_SXX:
			if(quantity==AVG_SXX) qid=XX;
		case AVG_SYY:
			if(quantity==AVG_SYY) qid=YY;
		case AVG_SXY:
			if(quantity==AVG_SXY) qid=XY;
			
			// Volume weighted average is Sum (Vp rhop stressp) / Sum Vp = Sum (mp stressp) / Sum Vp
			// where Vp = J mp/rho0
			for(p=0;p<nmpms;p++)
			{	matid=mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					mp = mpm[p]->mp;
                    Tensor sp = mpm[p]->ReadStressTensor();
 					value += mp*Tensor_i(&sp,qid);
					Vtot += Jp*mp/rho0;
				}
			}
			if(Vtot>0.) value /= Vtot;
			value *= UnitsController::Scaling(1.e-6);
			break;
		
		// Elastic strain (% in Legacy Units)
		// New method small strain = Biot strain - archived plastic strain
		// New method hyperelastic = Biot strain from elastic B in plastic strain
		// Membranes = 0
		case AVG_EZZE:
			qid=ZZ;
	    case AVG_EXZE:
			if(quantity==AVG_EXZE) qid=XZ;
	    case AVG_EYZE:
			if(quantity==AVG_EYZE) qid=YZ;
	    case AVG_EXXE:
			if(quantity==AVG_EXXE) qid=XX;
		case AVG_EYYE:
			if(quantity==AVG_EYYE) qid=YY;
		case AVG_EXYE:
			if(quantity==AVG_EXYE) qid=XY;
			
			// Volume weighted average is Sum (Vp strainp) / Sum Vp 
			// where Vp = J mp/rho0
			for(p=0;p<nmpms;p++)
			{	matid=mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					if(theMaterials[matid]->AltStrainContains()==ENG_BIOT_PLASTIC_STRAIN)
					{	// elastic strain = total strain minus plastic strain
						Matrix3 biot = mpm[p]->GetBiotStrain();
						Tensor *eplast=mpm[p]->GetAltStrainTensor();
						value += Vp*(biot.get(qid,2.) - Tensor_i(eplast,qid));
					}
					else if(theMaterials[matid]->AltStrainContains()==LEFT_CAUCHY_ELASTIC_B_STRAIN)
					{	// get elastic strain from elastic B in alt strain
						Matrix3 biot = mpm[p]->GetElasticBiotStrain();
						value += Vp*biot.get(qid,2.);
					}
					else
					{	// elastic strain = total strain for these materials
						Matrix3 biot = mpm[p]->GetBiotStrain();
						value += Vp*biot.get(qid,2.);
					}
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			value *= UnitsController::Scaling(100.);
			break;

		// plastic strain (% in Legacy)
		// New method small strain = archived plastic strain
		// New method hyperelastic = Biot strain from F - Biot strain from elastic B in plastic strain
		// Membrane = 0
		case AVG_EZZP:
			qid=ZZ;
		case AVG_EXZP:
			if(quantity==AVG_EXZP) qid=XZ;
		case AVG_EYZP:
			if(quantity==AVG_EYZP) qid=YZ;
		case AVG_EXXP:
			if(quantity==AVG_EXXP) qid=XX;
		case AVG_EYYP:
			if(quantity==AVG_EYYP) qid=YY;
		case AVG_EXYP:
			if(quantity==AVG_EXYP) qid=XY;
			
			// Volume weighted average is Sum (Vp strainp) / Sum Vp
			// where Vp = J mp/rho0
			for(p=0;p<nmpms;p++)
			{	matid=mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					if(theMaterials[matid]->AltStrainContains()==ENG_BIOT_PLASTIC_STRAIN)
					{	// plastic strain all ready
						Tensor *eplast=mpm[p]->GetAltStrainTensor();
						value += Vp*Tensor_i(eplast,qid);
					}
					else if(theMaterials[matid]->AltStrainContains()==LEFT_CAUCHY_ELASTIC_B_STRAIN)
					{	// plastic strain = total strain minus elastic strain (from B)
						Matrix3 biotTot = mpm[p]->GetBiotStrain();
						Matrix3 biotElastic = mpm[p]->GetElasticBiotStrain();
						value += Vp*(biotTot.get(qid,2.) - biotElastic.get(qid,2.)) ;
					}
					// others have zero plastic strain
					Vtot += Vp;;
				}
			}
			if(Vtot>0.) value /= Vtot;
			value *= UnitsController::Scaling(100.);
			break;

		// total strain (% in Legacy)
		// New method small strain = Biot strain from F
		// New method hyperelastic = Biot strain from F
		case AVG_EZZ:
			qid=ZZ;
		case AVG_EXZ:
			if(quantity==AVG_EXZ) qid=XZ;
		case AVG_EYZ:
			if(quantity==AVG_EYZ) qid=YZ;
		case AVG_EXX:
			if(quantity==AVG_EXX) qid=XX;
		case AVG_EYY:
			if(quantity==AVG_EYY) qid=YY;
		case AVG_EXY:
			if(quantity==AVG_EXY) qid=XY;
			
			// Volume weighted average is Sum (Vp strainp) / Sum Vp
			// where Vp = J mp/rho0
			for(p=0;p<nmpms;p++)
			{	matid=mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					Matrix3 biot = mpm[p]->GetBiotStrain();
					value += Vp*biot.get(qid,2.);
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			value *= UnitsController::Scaling(100.);
			break;
		
		// Deformation gradient
		case AVG_FZZ:
			qid=ZZ;
		case AVG_FXZ:
			if(quantity==AVG_FXZ) qid=XZ;
		case AVG_FYZ:
			if(quantity==AVG_FYZ) qid=YZ;
		case AVG_FXX:
			if(quantity==AVG_FXX) qid=XX;
		case AVG_FYY:
			if(quantity==AVG_FYY) qid=YY;
		case AVG_FXY:
			if(quantity==AVG_FXY) qid=XY;
		case AVG_FYX:
			if(quantity==AVG_FYX) qid=YX;
		case AVG_FZY:
			if(quantity==AVG_FZY) qid=ZY;
		case AVG_FZX:
			if(quantity==AVG_FZX) qid=ZX;
			
			// Volume weighted average is Sum (Vp strainp) / Sum Vp
			// where Vp = J mp/rho0
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					Matrix3 F = mpm[p]->GetDeformationGradientMatrix();
					value += Vp*F.get(qid,1.);
					Vtot += Vp;;
				}
			}
			if(Vtot>0.) value /= Vtot;
			value *= UnitsController::Scaling(100.);
			break;
			
		// totalenergies (Volume*energy) (J in Legacy)
		case KINE_ENERGY:
		case WORK_ENERGY:
		case STRAIN_ENERGY:
		case HEAT_ENERGY:
        case ENTROPY_ENERGY:
        case INTERNAL_ENERGY:
        case HELMHOLZ_ENERGY:
		{	bool threeD = fmobj->IsThreeD();
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	switch(quantity)
                    {   case KINE_ENERGY:
							value += 0.5*mpm[p]->mp*(mpm[p]->vel.x*mpm[p]->vel.x
																+ mpm[p]->vel.y*mpm[p]->vel.y);
							if(threeD)
								value += 0.5*mpm[p]->mp*(mpm[p]->vel.z*mpm[p]->vel.z);
                            break;
						case WORK_ENERGY:
							value += mpm[p]->mp*mpm[p]->GetWorkEnergy();
							break;
						case STRAIN_ENERGY:
							value += mpm[p]->mp*mpm[p]->GetStrainEnergy();
							break;
						case HEAT_ENERGY:
							value += mpm[p]->mp*mpm[p]->GetHeatEnergy();
							break;
						case ENTROPY_ENERGY:
							value += mpm[p]->mp*mpm[p]->GetEntropy();
                            break;
						case INTERNAL_ENERGY:
							value += mpm[p]->mp*(mpm[p]->GetWorkEnergy()+mpm[p]->GetHeatEnergy());
                            break;
						case HELMHOLZ_ENERGY:
							value += mpm[p]->mp*(mpm[p]->GetWorkEnergy()+mpm[p]->GetHeatEnergy()
                                                 - mpm[p]->pPreviousTemperature*mpm[p]->GetEntropy());
                            break;
						default:
							break;
					}
				}
			}
			value *= UnitsController::Scaling(1.e-9);
			break;
		}
		
		// interface energy (J in Legacy)
		case INTERFACE_ENERGY:
			value = NodalPoint::interfaceEnergy*UnitsController::Scaling(1.e-9);
			break;
			
		// fricitonal work (J in Legacy)
		case FRICTION_WORK:
			value = NodalPoint::frictionWork*UnitsController::Scaling(1.e-9);;
			break;
			
		// energies (Volume*energy) (J in Legacy)
		case PLAS_ENERGY:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
                {   value+=mpm[p]->mp*mpm[p]->GetPlastEnergy();
                }
			}
			value *= UnitsController::Scaling(1.e-9);
			break;
		
		// velocity x
		case AVG_VELX:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*mpm[p]->vel.x;
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
 			break;
			
		// velocity y
		case AVG_VELY:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*mpm[p]->vel.y;
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			break;
			
		// velocity z
		case AVG_VELZ:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*mpm[p]->vel.z;
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			break;
			
		// x displacement
		case AVG_DISPX:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*(mpm[p]->pos.x-mpm[p]->origpos.x);
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			break;
			
		// y displacement
		case AVG_DISPY:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*(mpm[p]->pos.y-mpm[p]->origpos.y);
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			break;
			
		// z displacement
		case AVG_DISPZ:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*(mpm[p]->pos.z-mpm[p]->origpos.z);
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			break;
			
		// temperature
		case AVG_TEMP:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*mpm[p]->pPreviousTemperature;
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			break;
		
		case WTFRACT_CONC:
		{	double totalWeight=0.;
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	double csat = mpm[p]->GetConcSaturation();
					value += mpm[p]->pPreviousConcentration*csat*mpm[p]->mp;
					totalWeight += mpm[p]->mp;
				}
			}
			if(totalWeight>0.) value /= totalWeight;
			break;
		}
		
		case STEP_NUMBER:
			value=(double)fmobj->mstep;
			break;
		
		// always in seconts
		case CPU_TIME:
			value=fmobj->CPUTime();
			break;

		// always in seconds
		case ELAPSED_TIME:
			value=fmobj->ElapsedTime();
			break;

		case GRID_ALPHA:
			value=bodyFrc.GetNonPICDamping(mtime);
			break;
		
		case PARTICLE_ALPHA:
			value=bodyFrc.GetNonPICParticleDamping(mtime);
			break;
            
		case HISTORY_VARIABLE:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	rho0 = theMaterials[matid]->GetRho(mpm[p]);
					Jp = theMaterials[matid]->GetCurrentRelativeVolume(mpm[p],0);
					Vp = Jp*mpm[p]->mp/rho0;
					value += Vp*theMaterials[mpm[p]->MatID()]->GetHistory(subcode,mpm[p]->GetHistoryPtr(0));
					Vtot += Vp;
				}
			}
			if(Vtot>0.) value /= Vtot;
			break;
		
		case TOT_FCONX:
		case TOT_FCONY:
		case TOT_FCONZ:
        {   // Three options
            //   1. VTK active, but not doing contact
            //   2. VTK inactive
            //   3. VTK active and archiving contact
            // When 1 and 2, all contact stuff here, which means must
            //     a. update lastArchiveContactStep, which is done in GetArchiveContactStepInterval
            //     b. clear force after reading
            //     c. Store last contact force in case another component is called next
            //     d. totalSteps will never be zero, so no need to track last contact force
            // When 3
            //     a. VTK archiving tracks lastArchiveContactStep
            //     b. Do not clear force (it is cleared on each VTK archive)
            //     c. VTK archiving will also save contact forces in case get here just after VTK archiving
            //          (since global archiving done after step is done
			
			// time steps since last cleared
			int totalSteps = archiver->GetArchiveContactStepInterval();
			
			// true if VTK archve inactive or it is not archiving contact on the grid
			bool clearForces = !archiver->GetDoingArchiveContact();
			
			// get array of vectors to store contact force for each material
			Vector *forces = archiver->GetLastContactForcePtr();
			
			// this vector will be filled
			Vector ftotal;
			ZeroVector(&ftotal);
			
			// if needed sum forces on all nodes
			if(totalSteps>0)
			{	// non-zero steps, may or may not be doing VTKArchive
				for(int im=0;im<maxMaterialFields;im++) ZeroVector(&forces[im]);
				double scale = -1./(double)totalSteps;
				for(p=1;p<=nnodes;p++)
				{	nd[p]->AddGetContactForce(clearForces,forces,scale,NULL);
				}
			}
			
			// extract proper force (sum or one value
			for(int im=0;im<maxMaterialFields;im++)
			{	if(whichMat==0)
				{	AddVector(&ftotal,&forces[im]);
				}
				else if(whichMat==MaterialBase::GetFieldMatID(im)+1)
				{	AddVector(&ftotal,&forces[im]);
					break;
			   }
			}
				
 			// pick the component
			if(quantity==TOT_FCONX)
				value=ftotal.x;
			else if(quantity==TOT_FCONY)
				value=ftotal.y;
			else
				value=ftotal.z;
			break;
		}
		
		case TOT_REACTX:
		case TOT_REACTY:
		case TOT_REACTZ:
		{	// find force for BCs with provided ID (N in Legacy)
			Vector freaction = NodalVelBC::TotalReactionForce(whichMat);
			
			// pick the component
			if(quantity==TOT_REACTX)
				value = freaction.x;
			else if(quantity==TOT_REACTY)
				value = freaction.y;
			else
				value = freaction.z;
			value *= UnitsController::Scaling(1.e-6);
			break;
		}
		
		case TOT_REACTQ:
		{	// find heat flow for BCs with provided ID (J in Legacy)
			double qreaction = NodalTempBC::TotalHeatReaction(whichMat);
			value = qreaction*UnitsController::Scaling(1.e-9);
			break;
		}
		
		// linear momentum (Legacy N-sec)
		case LINMOMX:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
					value += mpm[p]->mp*mpm[p]->vel.x;
			}
			value *= UnitsController::Scaling(1.e-6);
			break;
			
		case LINMOMY:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
					value += mpm[p]->mp*mpm[p]->vel.y;
			}
			value *= UnitsController::Scaling(1.e-6);
			break;
			
		case LINMOMZ:
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
					value += mpm[p]->mp*mpm[p]->vel.z;
			}
			value *= UnitsController::Scaling(1.e-6);
			break;
			
		// angular momentum (Legacy J-sec)
		case ANGMOMX:
		case ANGMOMY:
		case ANGMOMZ:
		{	Vector Ltot = MakeVector(0.,0.,0.);
			Vector cp;
			for(p=0;p<nmpms;p++)
			{	matid = mpm[p]->MatID();
				if(IncludeThisMaterial(matid))
				{	// get Mp Xp X Vp
					CrossProduct(&cp,&mpm[p]->pos,&mpm[p]->vel);
					AddScaledVector(&Ltot,&cp,mpm[p]->mp);
				}
			}
			if(quantity==ANGMOMX)
				value = Ltot.x;
			else if(quantity==ANGMOMY)
				value = Ltot.y;
			else
				value = Ltot.z;
			value *= UnitsController::Scaling(1.e-9);
			break;
		}

		// grid kinetic energy (J in Legacy)
		case GRID_KINE_ENERGY:
		{	double totalMass;
			for(p=1;p<=nnodes;p++)
				nd[p]->AddKineticEnergyAndMass(value,totalMass);
			value *= UnitsController::Scaling(1.e-9);
			break;
		}

		// skip unknown
		case UNKNOWN_QUANTITY:
			return nextGlobal;
		
		// zero if not programmed yet
		default:
			break;
	}
	
	toArchive.push_back(value);
	
	// return next one
	return nextGlobal;
}

// append tab and color string
GlobalQuantity *GlobalQuantity::AppendColor(char *fline)
{
	if(quantity==UNKNOWN_QUANTITY) return nextGlobal;
	
	strcat(fline,"\t");
	switch(colorID)
	{   case 0:
			strcat(fline,"black");
			break;
		case 1:
			strcat(fline,"blue");
			break;
		case 2:
			strcat(fline,"red");
			break;
		case 3:
			strcat(fline,"green");
			break;
		case 4:
			strcat(fline,"brown");
			break;
		case 5:
			strcat(fline,"cyan");
			break;
		case 6:
			strcat(fline,"magenta");
			break;
		case 7:
			strcat(fline,"orange");
			break;
		case 8:
			strcat(fline,"purple");
			break;
		case 9:
			strcat(fline,"yellow");
			break;
		default:
			strcat(fline,"black");
			break;
	}
	
	// return next one
	return nextGlobal;
}

#pragma mark GlobalQuantity::ACCESSORS

// decide if archiving this material
// return true is matches material (rigid or nonrigid), but of whichMat is zero
//      only return true if it is a non-rigid material. This one material can
//      average only any material, but average over all particles is non rigid only
bool GlobalQuantity::IncludeThisMaterial(int matid)
{
	// accept any specified material
	if(matid+1==whichMat) return (bool)true;
	
	// otherwise only allow non-rigid materials
	if(whichMat==0 && !theMaterials[matid]->Rigid()) return (bool)true;
	
	return (bool)false;
}

// set the next Global
GlobalQuantity *GlobalQuantity::GetNextGlobal(void) { return nextGlobal; }
void GlobalQuantity::SetNextGlobal(GlobalQuantity *newGlobal) { nextGlobal=newGlobal; }

// compare to settings
bool GlobalQuantity::IsSameQuantity(int qval,int qcode,int qmat)
{	if(quantity==qval && subcode==qcode && qmat==whichMat) return true;
	return false;
}

