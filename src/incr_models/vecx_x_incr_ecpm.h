/*
thot package for statistical machine translation
Copyright (C) 2013 Daniel Ortiz-Mart\'inez
 
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with this program; If not, see <http://www.gnu.org/licenses/>.
*/
 
/********************************************************************/
/*                                                                  */
/* Module: vecx_x_incr_ecpm                                         */
/*                                                                  */
/* Prototype file: vecx_x_incr_ecpm                                 */
/*                                                                  */
/* Description: Class to manage incremental encoded conditional     */
/*              probability models p(x|Vector<x>).                  */
/*                                                                  */
/********************************************************************/

#ifndef _vecx_x_incr_ecpm
#define _vecx_x_incr_ecpm

//--------------- Include files --------------------------------------

#if HAVE_CONFIG_H
#  include <thot_config.h>
#endif /* HAVE_CONFIG_H */

#include "vecx_x_incr_cptable.h"
#include "vecx_x_incr_enc.h"
#include "_incrEncCondProbModel.h"
#include "awkInputStream.h"

//--------------- Constants ------------------------------------------


//--------------- typedefs -------------------------------------------


//--------------- function declarations ------------------------------


//--------------- Classes --------------------------------------------

//--------------- vecx_x_incr_ecpm class

template<class HX,class X,class SRC_INFO,class SRCTRG_INFO>
class vecx_x_incr_ecpm: public _incrEncCondProbModel<Vector<HX>,HX,Vector<X>,X,SRC_INFO,SRCTRG_INFO>
{
 public:

  typedef typename _incrEncCondProbModel<Vector<HX>,HX,Vector<X>,X,SRC_INFO,SRCTRG_INFO>::SrcTableNode SrcTableNode;
  typedef typename _incrEncCondProbModel<Vector<HX>,HX,Vector<X>,X,SRC_INFO,SRCTRG_INFO>::TrgTableNode TrgTableNode;

      // Constructor
  vecx_x_incr_ecpm():_incrEncCondProbModel<Vector<HX>,HX,Vector<X>,X,SRC_INFO,SRCTRG_INFO>()
  {
    this->tablePtr=new vecx_x_incr_cptable<X,SRC_INFO,SRCTRG_INFO>;
    this->encPtr=new vecx_x_incr_enc<HX,X>;
  }

      // Functions to load and print the model
  bool load(const char *fileName);
  bool print(const char *fileName);
  ostream& print(ostream &outS);

      // Destructor
  ~vecx_x_incr_ecpm();
   
 protected:

};

//--------------- Template function definitions

//---------------
template<class HX,class X,class SRC_INFO,class SRCTRG_INFO>
bool vecx_x_incr_ecpm<HX,X,SRC_INFO,SRCTRG_INFO>::load(const char *fileName)
{
  Vector<HX> hs;
  HX ht;
  im_pair<SRC_INFO,SRCTRG_INFO> inf;
  awkInputStream awk;
  unsigned int i;

  if(awk.open(fileName)==ERROR)
  {
    cerr<<"Error while loading model file "<<fileName<<endl;
    return ERROR;
  }  
  else
  {
    cerr<<"Loading model file "<<fileName<<endl;

    this->tablePtr->clear();
    this->modelFileName=fileName;
    
    while(awk.getln())
    {
      if(awk.NF>1)
      {
        hs.clear();
        if(awk.NF>2)
        {
          for(i=1;i<awk.NF-2;++i)
          {
            hs.push_back(awk.dollar(i));
          }
        }
        ht=awk.dollar(awk.NF-2);
        inf.first=atof(awk.dollar(awk.NF-1).c_str());
        inf.second=atof(awk.dollar(awk.NF).c_str());     
        this->addTableEntryHigh(hs,ht,inf);
      }
    }
  }
  
  return OK;
}

//--------------
template<class HX,class X,class SRC_INFO,class SRCTRG_INFO>
bool vecx_x_incr_ecpm<HX,X,SRC_INFO,SRCTRG_INFO>::print(const char *fileName)
{
  ofstream outF;

  outF.open(fileName,ios::out);
  if(!outF)
  {
    cerr<<"Error while printing model to file."<<endl;
    return ERROR;
  }
  else
  {
    print(outF);
    outF.close();	
    return OK;
  }
}

//--------------
template<class HX,class X,class SRC_INFO,class SRCTRG_INFO>
ostream& vecx_x_incr_ecpm<HX,X,SRC_INFO,SRCTRG_INFO>::print(ostream &outS)
{
  Vector<HX> hs;
  Vector<X> s;
  HX ht;
  vecx_x_incr_cptable<X,SRC_INFO,SRCTRG_INFO>* tableCptPtr=0;
  unsigned int i;

  tableCptPtr=dynamic_cast<vecx_x_incr_cptable<X,SRC_INFO,SRCTRG_INFO>*>(this->tablePtr);
  
  if(tableCptPtr) // C++ RTTI
  {
    typename vecx_x_incr_cptable<X,SRC_INFO,SRCTRG_INFO>::const_iterator tableIter;
      
        // Set float precision.
    outS.setf( ios::fixed, ios::floatfield );
    outS.precision(8);
      
    for(tableIter=tableCptPtr->begin();tableIter!=tableCptPtr->end();++tableIter)
    {
      if(tableIter->first.size()>0 && (double)tableIter->second.get_c_st()>0)
      {
        s.clear();
        for(i=0;i<tableIter->first.size()-1;++i)
        {
          s.push_back(tableIter->first[i]);
        }
        this->Src_to_HighSrc(s,hs);
        this->Trg_to_HighTrg(tableIter->first.back(),ht);        

        for(i=0;i<hs.size();++i)
        {
          outS<<hs[i]<<" ";
        }
        bool found;
        outS<<ht<<" "<<this->getSrcInfo(s,found)<<" "<<tableIter->second<<endl;
      }
    }     
  }
  return outS;
}

//---------------
template<class HX,class X,class SRC_INFO,class SRCTRG_INFO>
vecx_x_incr_ecpm<HX,X,SRC_INFO,SRCTRG_INFO>::~vecx_x_incr_ecpm()
{
  
}

#endif
