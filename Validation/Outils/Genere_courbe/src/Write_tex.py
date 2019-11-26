#****************************************************************************
# Copyright (c) 2015 - 2016, CEA
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#*****************************************************************************

# -*- coding: latin-1 -*-

__version__ = '0.1'


from lib import chaine2Tex,chaine2File

from lib import GestionMessages

import sys, os, time


class FileTex:
    def __init__(self,name,mode):
        self.open(name,mode)
        pass
    def open(self,name,mode):
        self.file=open(name,mode)
        pass
    def write(self,titi):
        self.write_interne(titi)
        pass
    def write_interne(self,titi):
        self.file.write(titi)
        pass
    def write_text_sans_nl(self,titi):
        self.write_interne(chaine2Tex(titi))
        pass
    def write_text(self,titi):
        self.write_text_sans_nl(titi)
        self.write_interne('\n')
        pass
    def write_description_sans_nl(self,description):
        if (len(description)==0): return
        self.write_text_sans_nl(description[0])
        for i in range(1,len(description)):
            #self.write_Tex_sans_nl('\\\\')
            self.write_Tex('')
            self.write_Tex('')
            self.write_text_sans_nl(description[i])
            pass
        pass
    def write_description(self,description):
        self.write_description_sans_nl(description)
        self.write_interne('\n')
        pass

    def write_Tex_sans_nl(self,titi):
        titi2=chaine2File(titi)
        self.write_interne(titi2)
        pass
    def write_Tex(self,titi):
        self.write_Tex_sans_nl(titi)
        self.write_interne('\n')
        pass
    def close(self):
        self.file.close()
        pass
    pass
class Write_tex:
    def __init__(self,novisit=False):
        self.nb_visu=0
        self.minifigure_avt=0
        self.novisit = novisit
        pass

    def verifie_existance_fichier(self,nom_fichier,objet):
        iter = 0
        while iter<2 and (not os.path.isfile(nom_fichier)):
            objet.gestMsg.ecrire(GestionMessages._DEBOG, 'Search file %s from %s' % (nom_fichier, os.getcwd()))
            time.sleep(1)
            iter += 1
            # print 'waiting %d' % iter
            pass
        if not os.path.isfile(nom_fichier):
            objet.gestMsg.ecrire(GestionMessages._ERR, 'Error, graphic file %s is not generated ' % (nom_fichier))
            pass
        pass
    # inclusion de la figure dans le fichier latex
    def inclureFigureTex(self, figure,fichier):
        '''Inclusion de la figure dans le fichier latex du rapport de validation.'''
        minifigure=self.minifigure_avt
        if (figure.nb_img_without_newline>0):
            minifigure=1
            pass
        if (minifigure<1):
            fichier.write_Tex('\n')
            pass
        fichier.write_Tex('% Debut figure')
        if figure.titre!='Undefined':
            fichier.write_Tex('\subsection{%s}' % chaine2Tex(figure.titre))
            pass

        if minifigure:
            minipage_width=figure.width.split(",")[0]
            fichier.write_Tex('\hspace*\fill')
            fichier.write_Tex('\begin{minipage}{%s}{'%minipage_width)
            pass
        fichier.write_description(figure.description)
        fichier.write_text('')

        self.verifie_existance_fichier(figure.fichierGraphiqueComplet,figure)

        width="".join(figure.width.split())
        if (width!="0cm") and (width[0]!="-"):
            self.nb_visu+=1
            if figure.format=='ps' or  figure.format=='png' :
                fichier.write_Tex('\includegraphics[width=%s]{\orig/.tmp/%s}' % (figure.width,figure.fichierGraphique))
            else:
                # fichier.write('\input{%s}\n' % figure.fichierGraphiqueComplet)
                fichier.write_Tex('\input{\orig/.tmp/%s}' % figure.fichierGraphique)
                pass
            pass
        else:
            print("The figure does not really include the picture ",figure.fichierGraphique," because the width <= 0:",figure.width)
            pass
        if (minifigure==1):
            fichier.write_Tex('} \end{minipage}')
            fichier.write_Tex('\hspace*\fill')
            pass
        if (self.nb_visu>=figure.nb_img_without_newline):
            fichier.write_Tex('\n\n')
            self.nb_visu=0
            self.minifigure_avt=0
        else:
            self.minifigure_avt=minifigure
            pass
        pass
        if len(figure.listeCourbes)>0 and figure.inclureDescCourbes==1:
            # description des courbes
            fichier.write_Tex('\n% Debut courbe')
            fichier.write_text('Description des courbes de la figure %s:'%(figure.fichierGraphique) )
            fichier.write_Tex('\begin{itemize}')
            for courbe in figure.listeCourbes:
                #   courbe.inclureCourbeTex(fichier)
                fichier.write_Tex_sans_nl('\item %s : ' % (chaine2Tex(courbe.legende)))
                if len(courbe.description)!=0:
                    fichier.write_description_sans_nl(courbe.description)
                    fichier.write_Tex_sans_nl('\\\\')
                    pass
                tmp = False
                if courbe.origine!='Undefined':
                    fichier.write_text_sans_nl(courbe.origine)
                    tmp = True
                    pass
                if courbe.version!='Undefined':
                    fichier.write_text_sans_nl(' '+courbe.version)
                    tmp = True
                    pass
                if tmp: fichier.write_Tex_sans_nl('\\\\')

                if (courbe.fichier!='Undefined'):
                    fichier.write_text_sans_nl('\nfichier %s' % ((courbe.fichier)))
                    pass
                if (courbe.fonction!='Undefined'):
                    fichier.write_text_sans_nl('\nfonction %s' % ((courbe.fonction)))
                    pass
                pass
            fichier.write_Tex('\end{itemize}')
            fichier.write_Tex('')
            pass
        pass

    # inclusion de la visu dans le fichier latex
    def inclureVisuTex(self, visu,fichier):
        '''Inclusion de la visu dans le fichier latex du rapport de validation.'''
        fichier.write_Tex('% Debut visu')
        if visu.titre!='Undefined':
            fichier.write_Tex('\subsection{%s}' % chaine2Tex(visu.titre))
            pass
        minifigure=self.minifigure_avt
        if (visu.nb_img_without_newline>0):
            minifigure=1
            pass
        if minifigure:
            minipage_width=visu.width.split(",")[0]
            fichier.write_Tex('\hspace*\fill')
            fichier.write_Tex('\begin{minipage}{%s}{'%minipage_width)
            pass
        fichier.write_description(visu.description)
        fichier.write_text('')
        for i in range(max(len(visu.cycles.split()),1)):
            if (minifigure) and (i>0):
                fichier.write_Tex('\hspace*\fill')
                fichier.write_Tex('\begin{minipage}{%s}{'%minipage_width)
                pass
            if not self.novisit:
                fichiercomplet=visu.fichierGraphiqueComplet+'_'+str(i)+'.'+visu.format
                fichiergraphique=visu.fichierGraphique+'_'+str(i)+'.'+visu.format

                self.verifie_existance_fichier(fichiercomplet,visu)
                width="".join(visu.width.split())
                if (width!="0cm") and (width[0]!="-"):
                    self.nb_visu+=1
                    if visu.format=='ps' or visu.format=='png':
                        fichier.write_Tex('\includegraphics[width=%s]{\orig/.tmp/%s}' % (visu.width,fichiergraphique))
                    else:
                        fichier.write_Tex('\input{%s}' % fichiercomplet)
                else:
                    print("The visu does not really include the picture ",fichiergraphique," because the width <= 0 :",visu.width)
                    pass
            if (minifigure==1):
                fichier.write_Tex('} \end{minipage}')
                fichier.write_Tex('\hspace*\fill')
                pass
            if (self.nb_visu>=visu.nb_img_without_newline):
                fichier.write_Tex('\n\n')
                self.nb_visu=0
                self.minifigure_avt=0
            else:
                self.minifigure_avt=minifigure
                pass
            pass
        pass
    def inclureTableauTex(self,tableau,fichier):
        '''Inclusion du tableau dans le fichier latex du rapport de validation.'''
        Tableau_val=tableau.get_tableau()
        valeurT=tableau.label.split('|')
        label=[ x.strip() for x in valeurT ]
        fichier.write_Tex('\n% Debut tableau')
        if tableau.titre!='Undefined':
            fichier.write_Tex('\subsection{%s}' % chaine2Tex(tableau.titre))
            pass
        fichier.write_description(tableau.description)
        if len(tableau.listeLignes)==0: return
        nbc=tableau.nb_colonnes
        if (tableau.formule): nbc+=1
        data = [ ]

        if (tableau.label!="Undefined"):
            line = [ "", ]
            for i in range(nbc):
                line.append( chaine2Tex(label[i]) )
            data.append( line )
            # print "llll",line
            pass
        li=0
        # print "Tabbb" ,Tableau_val
        for ligne in tableau.listeLignes:
            values=Tableau_val[li]
            li=li+1
            # values=ligne.get_values(tableau.nb_colonnes,dico)
            # on charge toujours les valeurs mais on ne les affiches pas forcement
            from Ligne import Lignes
            if ligne.afficher_ligne:

                if (isinstance(ligne,Lignes)):
                    nb_ligne=len(values)/(nbc+1)
                    for ll in range(nb_ligne):
                        decal=(nbc+1)*ll
                        line = [ chaine2Tex(str(values[decal])), ]
                        for i in range(1,nbc+1):
                            line.append( chaine2Tex(str(values[i+decal])) )
                        data.append( line )
                        pass
                    pass
                else:
                    line = [ chaine2Tex(ligne.legende), ]
                    for i in range(nbc):
                        line.append( chaine2Tex(str(values[i] )) )
                    data.append( line )
                    pass
                pass
            pass

        nb_lines   = len(data)
        nb_columns = len( data[0] )

        if tableau.textsize:
            fichier.write_Tex(tableau.textsize+'\n')

        entete=('\n\begin{longtable}{|c|')

        nbl=nbc
        if tableau.transposed_display:
            nbl=nb_lines
            pass
        fichier.write_Tex('% '+str(nbl)+' '+str(nbc)+' '+str(len(tableau.listeLignes))+'\n')
        for i in range(nbl): entete+=('c|')
        entete+='}'
        fichier.write_Tex(entete)

        fichier.write_Tex_sans_nl('\hline \n ')



        for  line in data:
            assert( len(line) == nb_columns )
        if tableau.transposed_display:
            transposed_data=[]
            for i in range( nb_columns ):
                transposed_data.append( [ ] )

            for i in range( nb_lines ):
                for j in range( nb_columns ):
                    transposed_data[ j ].append( data[ i ][ j ] )

            for index, line in enumerate(transposed_data):
                fichier.write( ' & '.join( line ) )
                if index==0:
                    fichier.write(' \endhead  \hline \n')
                else:
                    fichier.write(' \\\\  \hline \n')
        else:
            for index, line in enumerate(data):
                fichier.write( ' & '.join( line ) )
                if index==0:
                    fichier.write(' \endhead  \hline \n')
                else:
                    fichier.write(' \\\\  \hline \n')
        fichier.write_Tex('\end{longtable}\n \n ')
        if tableau.textsize:
            fichier.write_Tex('\\normalsize\n')
        fichier.write_Tex_sans_nl('\vspace{0.5cm}')
        # print "ICICI gros travail a faire !!!!!!!"
        if len(tableau.listeLignes)>0 and tableau.inclureDescLignes==1:
            # description des lignes

            fichier.write_Tex('\n% Debut ligne')
            fichier.write_text('Description des lignes du tableau %s:'%((tableau.titre)) )
            fichier.write_Tex('\begin{itemize}')
            for ligne in tableau.listeLignes:
                if (isinstance(ligne,Lignes)):continue
                # inclusion de la ligne dans le fichier latex
                fichier.write_Tex_sans_nl('\item %s : ' % (chaine2Tex(ligne.legende)))
                if len(ligne.description)!=0:
                    fichier.write_description_sans_nl(ligne.description)
                    fichier.write_Tex_sans_nl('\\\\')
                    pass
                tmp = False
                if ligne.origine!='Undefined':
                    fichier.write_text_sans_nl('%s' % ((ligne.origine)))
                    tmp = True
                    pass
                if ligne.version!='Undefined':
                    fichier.write_text_sans_nl(' %s' % ((ligne.version)))
                    tmp = True
                    pass
                if tmp:
                    fichier.write('\\\\')
                    pass
                fichier.write_text_sans_nl('\nfichier %s' % ((ligne.fichier)))

                #  ligne.inclureLigneTex(fichier)
                pass
            fichier.write_Tex('\end{itemize}')
            fichier.write_Tex('')
            pass
        pass
    def inclureObjet(self,figure,fichier):
        from Visu import Visu
        from Figure import Figure
        from Tableau import Tableau
        if (isinstance(figure,Visu)):
            self.inclureVisuTex(figure,fichier)
        elif (isinstance(figure,Figure)):
            self.inclureFigureTex(figure,fichier)
        elif (isinstance(figure,Tableau)):
            self.inclureTableauTex(figure,fichier)
        else:
            figure.gestMsg.ecrire(GestionMessages._ERR, 'Unexpected figure type %s'%(figure))
            pass
        pass
    def inclureChapitreTex(self,chapitre):
        '''Inclusion du chapitre dans le fichier latex du rapport de validation.'''
        fichier=self.ficTex
        fichier.write_Tex('\n% Debut Chapitre')
        if chapitre.titre!='Undefined':
            fichier.write_Tex('\section{%s}' % chaine2Tex(chapitre.titre))
        fichier.write_description(chapitre.description)
        iter = 0

        #ajout des figures
        for figure in chapitre.listeFigures:
            self.inclureObjet(figure,fichier)
            pass
        pass
    def ecrire_fichier_maitre(self,maitre,dico):
        nomFichierTexComplet = dico["nomFichierTexComplet"]
        entete = self.get_template().replace('__TITRECAS__', chaine2Tex(maitre.titre))
        entete = entete.replace('__AUTEUR__', chaine2Tex(maitre.auteur))
        date = time.strftime('%d/%m/%Y')
        entete = entete.replace('__DATE__', chaine2Tex(date))

        ficTexm = FileTex(nomFichierTexComplet, 'w')
        ficTexm.write_Tex('\def\orig{..}')
        ficTexm.write_Tex(entete)

        ficTexm.write_Tex('\input{corps}')
        ficTexm.write_Tex('\n\n% Fin du document\n\end{document}')
        ficTexm.close()
        pass

    def write_liste_cas(self,maitre,ficTex):
        if maitre.casTest:
            ficTex.write_Tex('\par\subsection{Test cases}')
            # if 1:
            ficTex.write_Tex('\begin{itemize}')
            for cas in maitre.casTest:
                from lib import get_detail_cas
                nomcas,dir,nb_proc,comment=get_detail_cas(cas)
                ficData=nomcas+'.data'
                ficTex.write_Tex('\item %s : \textit{%s}' % (chaine2Tex(dir+"/"+ficData), chaine2Tex(comment)))
                pass
            ficTex.write_Tex('\end{itemize}')
            pass
        pass

    def write_liste_ref(self,maitre,ficTex):
        if  maitre.reference:
            ficTex.write_Tex('\par\subsection{References :}')
            ficTex.write_Tex('\begin{itemize}')
            for ref in maitre.reference:
                ficTex.write_Tex('\item %s' % (chaine2Tex(ref)))
                pass
            ficTex.write_Tex('\end{itemize}')
            pass
        pass

    def include_data(self,maitre,ficTex):
        if maitre.inclureData>0:
            first=1

            for cas in maitre.casTest:
                from lib import get_detail_cas
                nomcas,dir,nb_proc,comment=get_detail_cas(cas)
                ficData=nomcas+'.data'
                ficData = os.path.join(dir, ficData)
                # ficData2 = os.path.join('../', ficData)
                if ((maitre.inclureData==1) or ((maitre.inclureData==2) and (comment!="") )) :

                    if (first):
                        first=0
                        ficTex.write_Tex('\clearpage')
                        ficTex.write_Tex('\n\n% Inclusion des fichiers.data')
                        ficTex.write_Tex('\section{Data Files}')
                        ficTex.write_Tex('\lstdefinelanguage{triou}{\nmorecomment=[s]{\#}{\#}, morekeywords={vefprep1b, vdf, lire, lire_fichier, domaine, dimension, postraitement, fluide_incompressible, schema_euler_explicite, pb_hydraulique_turbulent, pb_hydraulique}, sensitive=false\n}')
                        ficTex.write_Tex('\lstset{\n basicstyle=\small, numbers=none, tabsize=2, extendedchars=true, linewidth=16cm, emptylines=0, language=triou\n}')
                        pass
                    print('-> File %s is included in the PDF report from %s' % (ficData, os.getcwd()))
                    # nomcas = get_nom_cas(cas[1])
                    ficTex.write_Tex('\subsection{%s}' % (chaine2Tex(nomcas)))
                    if os.path.isfile(ficData):
                        ficTex.write_Tex('\lstinputlisting{\orig/%s}' % (ficData))
                    else:
                        maitre.gestMsg.ecrire(GestionMessages._ERR, '-> Error, data file %s not found!' % (ficData))
                        pass
                    pass
                pass
            pass
        pass
    def write_fichiers(self,maitre,dico):
        # generation du fichier.tex
        destTMP = dico["destTMP"]
        self.ecrire_fichier_maitre(maitre,dico)

        ficTex = FileTex( destTMP + '/corps.tex', 'w')
        self.ficTex=ficTex
        self.ecrire_introduction(maitre,ficTex)
        self.write_liste_cas(maitre,ficTex)
        self.write_liste_ref(maitre,ficTex)

        for chapitre in maitre.listeChapitres:
            self.inclureChapitreTex(chapitre)
            pass

        # inclusion des fichiers.data
        self.include_data(maitre,ficTex)

        ficTex.close()

        pass
    def ecrire_introduction(self,maitre,ficTex):

        #suite_entete='''\\section{Introduction}\n\nValidation r\\'ealis\\'ee par : __AUTEUR__.\\\\Rapport g\\'en\\'er\\'e le __DATE__.\n'''
        #suite_entete = suite_entete.replace('__TITRECAS__', chaine2Tex(maitre.titre))
        #suite_entete = suite_entete.replace('__AUTEUR__', chaine2Tex(maitre.auteur))
        #suite_entete = suite_entete.replace('__DATE__', chaine2Tex(maitre.date))
        suite_entete='''\section{Introduction}\n'''
        ficTex.write_Tex(suite_entete)
        suite_entete='''Validation made by : __AUTEUR__.\\\\Report generated  __DATE__.'''
        # '
        suite_entete = suite_entete.replace('__AUTEUR__',maitre.auteur)
        import time
        date = time.strftime('%d/%m/%Y')
        suite_entete = suite_entete.replace('__DATE__',date)
        # suite_entete = suite_entete.replace('\\','\\\\')
        ficTex.write_text(suite_entete)
        ficTex.write_Tex('\par\subsection{Description}')
        ficTex.write_description(maitre.description)

        ficTex.write_Tex('\par\subsection{Parameters '+maitre.code+' }')
        ficTex.write_Tex('\begin{itemize}')
        version=maitre.versionTrioU
        ficTex.write_Tex('\item Version %s : %s' % (maitre.code,chaine2Tex(version)))
        try:
            from os import popen
            a=os.popen('egrep "code|version : " version_utilisee | awk \'{printf("%s " ,$NF)}\'') #| awk \'{print "\""$1 " ("$2")\""}\' `')
            chaine=a.read().split()
            version=chaine[0]+" ("+chaine[1]+")"
            ficTex.write_Tex('\item Version Trio\_U from out: %s' % chaine2Tex(version))
        except:
            pass

        for param in maitre.parametresTrioU:
            ficTex.write_Tex_sans_nl('\item ');ficTex.write_text(param)
            pass
        ficTex.write_Tex('\end{itemize}')
        pass
    def get_template(self):
        _templateTEX_ = '''% This file was generated automaticaly with the genererCources.py script
\documentclass[10pt,twoside,a4paper]{article}
\\usepackage[ascii]{}
\\usepackage{longtable}
\\usepackage[latin1]{inputenc}
% \\usepackage[french]{babel}
\\usepackage{amsmath,amssymb,amsfonts,textcomp}
\\usepackage{color}
% \\usepackage{calc}
% \\usepackage{hyperref}
% \hypersetup{colorlinks=true, linkcolor=blue, filecolor=blue, pagecolor=blue, urlcolor=blue}
\\usepackage{graphicx}
\\usepackage{fancyhdr}
% \\usepackage[pdfpagemode=FullScreen,bookmarksopen=true,bookmarks=true]{hyperref}
\\usepackage[bookmarksopen=true,bookmarks=true]{hyperref}

% \\usepackage{verbatim}
%\\usepackage{moreverb}
\\usepackage{listings}

\setlength\hoffset{0cm}
\setlength\voffset{0cm}
\setlength\oddsidemargin{0cm}
\setlength\evensidemargin{0cm}
\setlength\topmargin{0cm}
\setlength\headheight{1cm}
\setlength\headsep{0.2cm}
\setlength\marginparsep{0cm}
\setlength\marginparwidth{0cm}
\setlength\footskip{2cm}
\setlength\textwidth{16cm}
\setlength{\parindent}{0pt}

% Style page
\renewcommand{\sectionmark}[1]{\markboth{#1}{#1}}
\pagestyle{fancy}
\lhead{\leftmark\\\\\rightmark}
\chead{}
\rhead{}
%\lfoot{\textit{Genere automatiquement avec genererCourbe.py, v'''+__version__+'''.}}
\lfoot{\textit{ __TITRECAS__}}
\cfoot{}
\rfoot{\thepage}
\renewcommand{\headrulewidth}{0.4pt}
\renewcommand{\footrulewidth}{0.4pt}

\title{__TITRECAS__}
\author{__AUTEUR__}
\date{__DATE__}

\makeindex

% Debut document
\begin{document}
{\huge\centering __TITRECAS__ \par}
'''
        return _templateTEX_
