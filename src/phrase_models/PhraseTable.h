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
/* Module: PhraseTable                                              */
/*                                                                  */
/* Prototype file: PhraseTable                                      */
/*                                                                  */
/* Description: Implements a bilingual phrase table.                */
/*                                                                  */
/********************************************************************/

#ifndef _PhraseTable
#define _PhraseTable

//--------------- Include files --------------------------------------

#if HAVE_CONFIG_H
#  include <thot_config.h>
#endif /* HAVE_CONFIG_H */

#include "BasePhraseTable.h"
#include "PhraseDict.h"
#include "PhraseCounts.h"

//--------------- Constants ------------------------------------------


//--------------- typedefs -------------------------------------------


//--------------- function declarations ------------------------------


//--------------- Classes --------------------------------------------


//--------------- PhraseTable class

class PhraseTable: public BasePhraseTable
{
 public:

    typedef std::map<Vector<WordIndex>,PhrasePairInfo> SrcTableNode;
    typedef std::map<Vector<WordIndex>,PhrasePairInfo> TrgTableNode;

      // Constructor
    PhraseTable(void);

        // Abstract function definitions
    virtual void addTableEntry(const Vector<WordIndex>& s,
                               const Vector<WordIndex>& t,
                               PhrasePairInfo inf);
        // Adds an entry to the probability table
    virtual void addSrcInfo(const Vector<WordIndex>& s,Count s_inf);
    virtual void addSrcTrgInfo(const Vector<WordIndex>& s,
                               const Vector<WordIndex>& t,
                               Count st_inf);
    virtual void incrCountsOfEntry(const Vector<WordIndex>& s,
                                   const Vector<WordIndex>& t,
                                   Count c);
        // Increase the counts of a given phrase pair
    virtual PhrasePairInfo infSrcTrg(const Vector<WordIndex>& s,
                                     const Vector<WordIndex>& t,
                                     bool& found);
        // Returns information related to a given s and t.
    virtual Count getSrcInfo(const Vector<WordIndex>& s,bool &found);
        // Returns information related to a given s and t.
    virtual Count getSrcTrgInfo(const Vector<WordIndex>& s,
                                const Vector<WordIndex>& t,
                                bool &found);
        // Returns information related to a given s and t.
    virtual Prob pTrgGivenSrc(const Vector<WordIndex>& s,
                              const Vector<WordIndex>& t);
    virtual LgProb logpTrgGivenSrc(const Vector<WordIndex>& s,
                                   const Vector<WordIndex>& t);
    virtual Prob pSrcGivenTrg(const Vector<WordIndex>& s,
                              const Vector<WordIndex>& t);
    virtual LgProb logpSrcGivenTrg(const Vector<WordIndex>& s,
                                   const Vector<WordIndex>& t);
    virtual bool getEntriesForTarget(const Vector<WordIndex>& t,
                                     SrcTableNode& srctn);
        // Stores in srctn the entries associated to a given target
        // phrase t, returns true if there are one or more entries
    virtual bool getEntriesForSource(const Vector<WordIndex>& s,
                                     TrgTableNode& trgtn);
        // Stores in trgtn the entries associated to a given source
        // phrase s, returns true if there are one or more entries
    virtual bool getNbestForSrc(const Vector<WordIndex>& s,
                                NbestTableNode<PhraseTransTableNodeData>& nbt);
    virtual bool getNbestForTrg(const Vector<WordIndex>& t,
                                NbestTableNode<PhraseTransTableNodeData>& nbt,
                                int N=-1);

       // Counts-related functions
    virtual Count cSrcTrg(const Vector<WordIndex>& s,
                          const Vector<WordIndex>& t);
    virtual Count cSrc(const Vector<WordIndex>& s);
    virtual Count cTrg(const Vector<WordIndex>& t);

        // Additional Functions
    bool nodeForTrgHasAtLeastOneTrans(const Vector<WordIndex>& t);
        // Returns true if t has one translation or more
    
        // size and clear functions
    virtual size_t size(void);
    virtual void clear(void);   

        // Destructor
    virtual ~PhraseTable();

      // const_iterator
    class const_iterator;
    friend class const_iterator;
    class const_iterator
      {
        protected:
           const PhraseTable* ptPtr;
           PhraseDict::const_iterator pdIter;
           
        public:
           const_iterator(void){ptPtr=NULL;}
           const_iterator(const PhraseTable* _ptPtr,
                          PhraseDict::const_iterator iter):ptPtr(_ptPtr),pdIter(iter)
             {
             }
           bool operator++(void); //prefix
           bool operator++(int);  //postfix
           int operator==(const const_iterator& right); 
           int operator!=(const const_iterator& right); 
           const PhraseDict::const_iterator& operator->(void)const;
      };

        // const_iterator related functions
    PhraseTable::const_iterator begin(void)const;
    PhraseTable::const_iterator end(void)const;
	
 protected:
	PhraseDict phraseDict;
    PhraseCounts sourcePhraseCounts;

    void getPhraseGivenState(PhraseCountState ps_state,
                             Vector<WordIndex>& s_phrase);
    Count getCountGivenState(PhraseCountState ps_state);
    bool getEntriesForTarget(PhraseTableNode* ptnPtr,
                             SrcTableNode& srctn); 
        // The same as the public getEntriesforTarget() function but a
        // pointer to a PhraseTableNode object is given (which is
        // provided by the nodeForTrgHasOneTransOrMore function)
    pair<bool,PhraseTableNode*>
      nodeForTrgHasOneTransOrMore(const Vector<WordIndex>& t);
        // Returns true if the target phrase t has one translation or
        // more and a pointer to the corresponding translations

};

#endif
