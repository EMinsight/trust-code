#!/usr/bin/env python3

"""
Update TRUST dataset to move medium block inside of problem, along with gravity.

Authors: A Bruneton, E Saikali
"""

from tparser import TRUSTParser, TObject

# All keywords below in lower case!!
PB_LIST = ["pb_conduction",
           "pb_phase_field",
           "pb_hydraulique", 
           "pb_hydraulique_melange_binaire_qc",
           "pb_hydraulique_melange_binaire_wc",
           "pb_hydraulique_ale",
           "pb_hydraulique_aposteriori",
           "pb_thermohydraulique", 
           "pb_thermohydraulique_qc", 
           "pb_thermohydraulique_wc",
           "pb_thermohydraulique_especes_wc",
           "pb_hydraulique_turbulent",
           "pb_hydraulique_turbulent_qc",
           "pb_hydraulique_turbulent_wc",
           "pb_thermohydraulique_turbulent",
           "pb_thermohydraulique_turbulent_qc",
           "pb_thermohydraulique_turbulent_wc",
           "probleme_diphasique_4eqs",
           "pb_conduction_combustible1d"] # exclude pb_couple, pb_multiphase, pb_med
MIL_LIST = ["solide", 
            "fluide_incompressible", 
            "fluide_quasi_compressible", 
            "fluide_weakly_compressible",
            "milieu_diphasique_1"]
GRAVITE = ["gravite", "gravity"] # Strong assumption, gravity field is always called gravite/gravity

#
# Utility functions
#
def is_float(s):
    try:
        float(s)
        return True
    except:
        return False

#
# Small container classes:
#
class Milieu(TObject):
    def __init__(self):
        super().__init__()
        self.assPb = -1     # Token index of the 'associate' directive (with pb)
        self.assGrav = -1   # Token index of the gravity 'associate' directive
        self.poro = {}      # Token indices of a possible "porosites_champs" directive
        self.poroSurf = {}  # Token indices of a possible "porosite_surf_champ" directive
        self.diamHyd = {}   # Token indices of a possible "diametre_hyd_champ" directive
        self.newText = []
        
class Problem(TObject):
    def __init__(self):
        super().__init__()
        self.medName = "" # Name of the associated medium
    
class Gravite(TObject):
    def __init__(self):
        super().__init__()
        self.newText = []

class MilieuModifier(TRUSTParser):
    """ Main class.
    """
    def __init__(self):
        super().__init__()
        self.mediums = {}      # A dictionnary of Milieu()
        self.problems = {}      # A dictionnary of Problem()
        self.gravite = Gravite()  # Gravity field if any.
   
    def checkName(self, name):
        """ Check if the name is a valid medium name, otherwise probably means that we are
        inspecting the medium block which is already inside the problem
        """
        if name == "{" or name == "#" or name == "/":
            return False
        return True
   
    def identifyProblems(self):
        """ Find all problems in data file
        """
        pbName = ""
        for p in PB_LIST: 
            off = 0
            while True: 
                pbName, idx = self.getObjName(p, off)
                if pbName == "": break
                off = idx+1
                pb = Problem()
                pb.name = pbName
                pb.start = self.getNext(self.findReadBlock(pbName), 2)
                self.problems[pbName] = pb   
   
    def _completeSingleMedium(self, mil):
        """ Complete various indices for a single medium
        """
        # Identify begining and end of block:
        mil.start = self.findReadBlock(mil.name)
        if mil.start == -1:
            raise ValueError(f"WHAAAT? Milieu {milName} is declared but never read ... fix this NOW!")
        e = self.findBlockEnd(self.getNext(mil.start, 2))
        mil.end = self.getNextJustAfter(e,1)

        # Find associate - NB: only one associate supported
        mil.assPb = self.findAssociate(mil.name)
        if mil.assPb == -1:
            raise ValueError("WHAAAT? No medium association found for milieu %s ..." % mil.name)
        # Read associated pb
        pb = self.tabToken[self.getNext(mil.assPb, 1)]
        if pb not in self.problems:
            print("!!Warning!! Medium association might be buggy, or problem type is not supported (Front-Tracking?) ! Problem '%s' not found/covered ..." % pb)
            return None
        self.problems[pb].medName = mil.name
        return mil

    def identifyMilieu(self):
        """ Locate declaration, reading and association of all media in the data file
        """
        # Spot various milieu blocks of data
        for m in MIL_LIST:
            idx = 0
            # We can have several medium with the same type:
            while idx != -1:
                milName, idx = self.getObjName(m, idx)
                if milName != "":
                    if not self.checkName(milName):
                        print("It seems that your datafile is already in the new 'milieu' syntax!")
                        return False
                    mil = Milieu()
                    mil.name = milName
                    mil.type = m
                    mil.decl = idx
                    mil = self._completeSingleMedium(mil)
                    # Skip some types of problems, like FT:
                    idx = idx + 1
                    if mil is None:
                        continue
                    self.mediums[milName] = mil
                    print("Milieu '%s' of type '%s' found at index %d" % (mil.name, m, idx))
                    
        if len(self.mediums) == 0:
            print("No 'milieu' found!!")
            return False
        return True
      
    def identifyGravity(self):
        """ Find out position (if any) of gravity field in dataset
        """
        g = self.gravite
        grav_nam = ""
        for gn in GRAVITE:
            try:
                g.decl = self.tabTokenLow.index(gn)
                g.name = gn
                break
            except:
                pass
        if g.decl == -1:
            print("Gravity field not found!")
            self.gravite = None
            return
        g.decl = self.getNext(g.decl, -1) # Capture field type like 'Champ_front_uniforme' written before keyword 'gravite'
        g.type = self.tabTokenLow[g.decl]

        g.start = self.findReadBlock(g.name)
        dim = self.readDimension()
        if dim == -1:  # No dimension keyword in dataset; maybe included in another dataset
            dim = int(self.tabToken[self.getNext(g.start,2)])
            print("Guessing dimension from gravite: ", dim)
        g.end = self.getNextJustAfter(g.start, 3+dim)
        
        # Find all 'associate' directives for gravity
        ass, found, idx = -2, False, 0
        while ass != -1:
            ass = self.findAssociate(g.name, idx)
            if ass != -1:
                found = True
                milNam = self.tabToken[self.getNext(ass, 1)]
                if milNam.lower() == "solide":
                    raise ValueError("WHAAAT? Gravity is associated to 'solide' medium... this does not make sense!! Please fix.")
                if milNam not in self.mediums:
                    raise ValueError("Error: milieu name in 'associate' is not matching for gravity")
                mil = self.mediums[milNam]
                mil.assGrav = ass
                idx = ass+1
                print(f"Milieu '{milNam}' has associated gravity.")
        # No gravity association found, gravity is irrelevant:
        if not found:  
            g = None

    def _identifyGen(self, kw):
        """ Identify and create the new text for 'porosites_champs' and/or 'porosite_surf_champ' 
            and/or 'diametre_hyd_champ'
        """
        pbNam, idx = self.getObjName(kw)
        if idx != -1:
            if pbNam not in self.problems:
                raise ValueError(f"Error handling keyword '{kw}': associated problem '{pbNam}' not found!")
            milNam = self.problems[pbNam].medName
            mil = self.mediums.get(milNam, None)
            if mil is None:
                raise ValueError(f"Error handling keyword '{kw}': associated problem '{pbNam}' has no associated medium!")
            # Not proud of this:
            att = {"porosites_champ": mil.poro,
                   "porosite_surf_champ": mil.poroSurf,
                   "diametre_hyd_champ": mil.diamHyd }[kw]
            att["start"] = idx
            # This one is easy: always 3 tokens
            if kw == "porosite_surf_champ": 
                att["end"] = self.getNextJustAfter(idx, 2+3)
            else:
                #
                # HACK HACK!! Find end of field instanciation ... hiiic. Number of tokens depend on field type ...
                # We look for the last float or some other magical heuristic ...
                #
                idx = self.getNext(idx, 2)
                fld_typ = self.tabTokenLow[idx]
                print(fld_typ)
                idx = self.getNext(idx, 1)
                if self.tabTokenLow[idx] == '{':
                    att["end"] = self.getNextJustAfter(self.findBlockEnd(idx), 1)
                else:
                    idx = self.getNext(idx, 1)  # at least 2 tokens in a field instanciation
                    # For champ_uniforme_morceaux, we might still have a '{' ... so try a little harder:
                    if self.tabTokenLow[idx] == '{':
                        att["end"] = self.getNextJustAfter(self.findBlockEnd(idx), 1)
                        return True
                    idx2 = self.getNext(idx, 1)
                    if self.tabTokenLow[idx2] == '{':
                        att["end"] = self.getNextJustAfter(self.findBlockEnd(idx2), 1)
                        return True
                    
                    if fld_typ in ["champ_fonc_fonction", "champ_fonc_xyz", "champ_fonc_txyz"]:
                        nb_tok = int(self.tabTokenLow[idx])
                        print(nb_tok)
                        idx = self.getNext(idx, nb_tok)
                        att["end"] = self.getNextJustAfter(idx, 1)
                        return True
                    
                    # Last resort, look for last float ...
                    while not is_float(self.tabTokenLow[idx]):  
                        idx = self.getNext(idx, 1)
                    while is_float(self.tabTokenLow[idx]):
                        idx = self.getNext(idx, 1)
                    # Rewind to catch last float:
                    idx = self.getNext(idx, -1)
                    att["end"] = self.getNextJustAfter(idx,  1)
            return True
        else:
            return False

    def identifyPorosite(self):
        return self._identifyGen("porosites_champ")

    def identifyPorositeSurf(self):
        return self._identifyGen("porosite_surf_champ")
    
    def identifyDiamHyd(self):
        return self._identifyGen("diametre_hyd_champ")
          
    def indent(self, tkLst):
        """ Indent some lines """
        res = []
        for t in tkLst:
            if t.endswith("\n"): 
                res.append(t)
                res.append("  ")
            else:
                res.append(t)  
        return res

    def createNewGravity(self):
        """ Produce piece of text that will go into each medium for gravity
        """
        gravTxt = [""]
        g = self.gravite
        if not g is None:
            startDeclG = self.getNext(g.start, 2)
            gravTxt = ["     gravite", g.type] + self.tabToken[startDeclG:g.end]
            #print("Gravite: ", ' '.join(gravTxt))
            g.newText = gravTxt    
            
    def createExtraFields(self, medium):
        """ Create new text for extra entries in medium, like porosites_champs, etc ...
        """
        tt = self.tabToken
        ret = []
        for att, kw in zip([medium.poro, medium.poroSurf, medium.diamHyd], 
                           ["porosites_champ", "porosite_surf_champ", "diametre_hyd_champ"]):
            if len(att):
                s, e = att["start"], att["end"]
                s2 = self.getNext(s, 2)
                # Strip last token to handle "\n" properly and only have a single line return:
                ret.extend(["    ", kw] + tt[s2:e-1] + [tt[e-1].strip(), "\n"])
        return ret

    def createNewMediumBlocks(self):
        """ Create all new medium blocks that will be inserted into the problems
        """
        gravTxt = self.gravite.newText if not self.gravite is None else [""]
        tt = self.tabToken
        for mName, m in self.mediums.items():
            gt = gravTxt if m.assGrav != -1 else [""]
            extra = self.createExtraFields(m)
            justAfterAcc = self.getNextJustAfter(m.start, 3)  # After the '{' in 'read mil ...'
            milImplI = self.getNextJustAfter(m.start, 2)  # skip 'read mil'
            mil = ['\n', m.type] + tt[milImplI:justAfterAcc] + gt + extra + tt[justAfterAcc:m.end] + ['\n']
            mil = self.indent(mil)
            #print("Milieu: ", ' '.join(mil))
            self.mediums[mName].newText = mil
          
    def outputData(self, fNameO):
        """ Produce final output file.
        """
        tt = self.tabToken
        # Things we will need to skip in newly created data file
        skip = []
        # Skip gravite declaration and instanciation:
        if not self.gravite is None:
            g = self.gravite
            skip.extend([(g.decl,self.getNextJustAfter(g.decl, 2)),
                         (g.start,g.end)])
        for _, m in self.mediums.items():
            skip.extend([(m.decl,self.getNextJustAfter(m.decl, 2)),
                         (m.start,m.end),
                         (m.assPb,self.getNextJustAfter(m.assPb, 3))
                        ])
            # Also skip association betw milieu and gravite if it is there:
            if m.assGrav != -1:
                skip.append((m.assGrav,self.getNextJustAfter(m.assGrav, 3)))
            # And finally skip porosites_champs, etc ... if there:
            for att in [m.poro, m.poroSurf, m.diamHyd]:
                if len(att) == 0: continue
                skip.append((att["start"], att["end"]))
                
        # Be sure to remove various blocks in the right order:
        skip.sort()

        # Create final token list by skiping what's given in 'skip' and inserting medium in problem:
        newData = tt[:skip[0][0]]           
        prev = skip[0][1]
        for ss, se in skip:
            newData.extend(tt[prev:ss])
            prev = se
        
        # Now insert milieu blocks into relevant problem:
        # Sort problems according to their start indices, so we proceed in order:
        pbWithStart = [(p.start, p) for p in self.problems.values()]
        pbWithStart.sort()
    
        # For each pb, move/copy 'milieu' block inside its 'read' part:
        prev = se
        for pbIdxStart, pb in pbWithStart:
            pbStartInside = self.getNextJustAfter(pbIdxStart, 1)  # After the '{' in the initial 'read pb', i.e. first token for first equation.
            newData.extend(tt[prev:pbStartInside])
            # Find correct medium
            mil = self.mediums[pb.medName]   # All checks done before, should always be ok            
            # Put milieu inside pb:
            newData.extend(mil.newText)
            prev = pbStartInside
        # Finish writing:
        newData.extend(tt[pbStartInside:])
        self.unTokenizeAndWrite(newData, fNameO)
            
if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        print("Usage: convert_milieu.py <input_file.data> <output_file.data>")
        sys.exit(-1)
    fNameI, fNameO = sys.argv[1], sys.argv[2]
    dm = MilieuModifier()
    dm.readAndTokenize(fNameI)
    dm.identifyProblems()
    if dm.identifyMilieu():
        dm.identifyGravity()
        dm.createNewGravity()
        dm.identifyPorosite()
        dm.identifyPorositeSurf()
        dm.identifyDiamHyd()
        dm.createNewMediumBlocks()
        dm.outputData(fNameO)
        print("File '%s' written!" % fNameO)
    else:
        print("Nothing done.")
        sys.exit(-1)
    
