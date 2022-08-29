/****************************************************************************
* Copyright (c) 2022, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <Bords.h>

Implemente_liste(Bord);
Implemente_instanciable(Bords,"Bords",LIST(Bord));


/*! @brief Ecrit tous les bords de la liste sur un flot de sortie.
 *
 * @param (Sortie& os) un flot de sortie
 * @return (Sortie&) le flot de sortie modifie
 */
Sortie& Bords::printOn(Sortie& os) const
{
  return LIST(Bord)::printOn(os);
}


/*! @brief Lit les bords a partir d'un flot d'entree.
 *
 * @param (Entree& is) un flot d'entree
 * @return (Entree&) le flot d'entree modifie
 */
Entree& Bords::readOn(Entree& is)
{
  return LIST(Bord)::readOn(is);
}


/*! @brief Associe une zone a tous les bords de la liste.
 *
 * @param (Zone& une_zone) la zone a associer aux bords de la liste
 */
void Bords::associer_zone(const Zone& une_zone)
{
  LIST_CURSEUR(Bord) curseur(*this);
  while(curseur)
    {
      curseur->associer_zone(une_zone);
      ++curseur;
    }
}


/*! @brief Renvoie le nombre total de faces de tous les bords de la liste
 *
 * @return (int) le nombre total de faces de tous les bords de la liste
 */
int Bords::nb_faces() const
{
  CONST_LIST_CURSEUR(Bord) curseur(*this);
  int nombre=0;
  while(curseur)
    {
      nombre+=curseur->nb_faces();
      ++curseur;
    }
  return nombre;
}

/*! @brief Renvoie le nombre total de faces du type specifie, pour tous les bords de la liste.
 *
 * @param (Type_Face type) le type des faces a comptabiliser
 * @return (int) le nombre total de faces du type specifie, pour tous les bords de la liste
 */
int Bords::nb_faces(Type_Face type) const
{
  CONST_LIST_CURSEUR(Bord) curseur(*this);
  int nombre=0;
  while(curseur)
    {
      if (type == curseur->faces().type_face())
        nombre+=curseur->nb_faces();
      ++curseur;
    }
  return nombre;
}
