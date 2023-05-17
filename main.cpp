/**
* Programme qui affiche les noms de différents jeux et designers en effectuant de l'allocation dynamique de mémoire.
* \file main.cpp
* \authors Emlyn Murphy et Maya Manel Ait Djebara
* \date 16 mai 2023
* Créé le 16 mai 2023
*/

#include "Jeu.hpp"
#include <iostream>
#include <fstream>
#include "cppitertools/range.hpp"
#include "gsl/span"
#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp"
#include "debogage_memoire.hpp"  //NOTE: Incompatible avec le "placement new", ne pas utiliser cette entête si vous utilisez ce type de "new" dans les lignes qui suivent cette inclusion.

using namespace std;
using namespace iter;
using namespace gsl;

#pragma region "Fonctions de base pour vous aider"
typedef uint8_t UInt8;
typedef uint16_t UInt16;
UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}
gsl::span<Jeu*> spanListeJeux(const ListeJeux& liste)
{
	return gsl::span(liste.elements, liste.nElements);
}
gsl::span<Designer*> spanListeDesigners(const ListeDesigners& liste)
{
	return gsl::span(liste.elements, liste.nElements);
}
#pragma endregion

//TODO: Fonction qui cherche un designer par son nom dans une ListeJeux.
// Cette fonction renvoie le pointeur vers le designer si elle le trouve dans
// un des jeux de la ListeJeux. En cas contraire, elle renvoie un pointeur nul.
Designer* trouverDesigner(ListeJeux liste, string nomDesigner)
{
	Designer* resultat = nullptr;
	for (Jeu* jeu : spanListeJeux(liste))
	{
		for (Designer* designer : spanListeDesigners(jeu->designers))
		{
			if (designer->nom == nomDesigner)
			{
				resultat = designer;
			}
		}
	}
	return resultat;
}

Designer* lireDesigner(istream& fichier, ListeJeux& listePrincipale)
{
	Designer designer = {}; // On initialise une structure vide de type Designer.
	designer.nom = lireString(fichier);
	designer.anneeNaissance = lireUint16(fichier);
	designer.pays = lireString(fichier);
	// Rendu ici, les champs précédents de la structure designer sont remplis avec la bonne information.

	//TODO: Ajouter en mémoire le designer lu. Il faut revoyer le pointeur créé.
	// Attention, valider si le designer existe déjà avant de le créer, sinon
	// on va avoir des doublons car plusieurs jeux ont des designers en commun
	// dans le fichier binaire. Pour ce faire, cette fonction aura besoin de
	// la liste de jeux principale en paramètre.
	Designer* designerExistant = trouverDesigner(listePrincipale, designer.nom);
	if (designerExistant != nullptr)
	{
		return designerExistant;
	}
	// Afficher un message lorsque l'allocation du designer est réussie.
	//cout << designer.nom << endl;  //TODO: Enlever cet affichage temporaire servant à voir que le code fourni lit bien les jeux.	else {
	else
	{
		int capaciteInitiale = 1;
		Jeu** nouveauDesigner = new Jeu* [capaciteInitiale];
		ListeJeux listeJeuxParticipes = { capaciteInitiale, 0, nouveauDesigner };
		designer.listeJeuxParticipes = listeJeuxParticipes;
		return &designer; //TODO: Retourner le pointeur vers le designer crée.
	}
}

//TODO: Fonction qui change la taille du tableau de jeux de ListeJeux.
// Cette fonction doit recevoir en paramètre la nouvelle capacité du nouveau
// tableau. Il faut allouer un nouveau tableau assez grand, copier ce qu'il y
// avait dans l'ancien, et éliminer l'ancien trop petit. N'oubliez pas, on copie
// des pointeurs de jeux. Il n'y a donc aucune nouvelle allocation de jeu ici !
void changerTailleListe(ListeJeux& liste, int nouvelleCapacite)
{	
	if (liste.capacite == liste.nElements)
	{
		Jeu** nouvelleListe;
		nouvelleListe = new Jeu * [nouvelleCapacite];
		for (int i = 0; i < liste.capacite; i++)
		{
			nouvelleListe[i] = liste.elements[i];
		}
		delete[] liste.elements;
		liste.elements = nouvelleListe;
		liste.capacite = nouvelleCapacite;
	}
}

//TODO: Fonction pour ajouter un Jeu à ListeJeux.
// Le jeu existant déjà en mémoire, on veut uniquement ajouter le pointeur vers
// le jeu existant. De plus, en cas de saturation du tableau elements, cette
// fonction doit doubler la taille du tableau elements de ListeJeux.
// Utilisez la fonction pour changer la taille du tableau écrite plus haut.
void ajouterJeu(ListeJeux& liste, Jeu* jeu)
{
	int nouvelleCapacite = liste.capacite * 2;
	changerTailleListe(liste, nouvelleCapacite);
	liste.elements[liste.nElements] = jeu;
	liste.nElements++;
}

//TODO: Fonction qui enlève un jeu de ListeJeux.
// Attention, ici il n'a pas de désallocation de mémoire. Elle enlève le
// pointeur de la ListeJeux, mais le jeu pointé existe encore en mémoire.
// Puisque l'ordre de la ListeJeux n'a pas être conservé, on peut remplacer le
// jeu à être retiré par celui présent en fin de liste et décrémenter la taille
// de celle-ci.
void enleverJeu(ListeJeux& liste, Jeu* jeu)
{
	int nouvelleCapacite = liste.capacite;
	Jeu** nouvelleListe = new Jeu * [nouvelleCapacite];
	int compteur = 0;
	for (int i = 0; i < liste.nElements; i++)
	{
		if (liste.elements[i] != jeu)
		{
			nouvelleListe[compteur] = liste.elements[i];
			compteur++;
		}
	}
	delete[] liste.elements;
	liste.elements = nouvelleListe;
	liste.nElements--;
}

Jeu* lireJeu(istream& fichier, ListeJeux& listePrincipale)
{
	Jeu* jeu = new Jeu();
	//Jeu jeu = {}; // On initialise une structure vide de type Jeu
	jeu->titre = lireString(fichier);
	jeu->anneeSortie = lireUint16(fichier);
	jeu->developpeur = lireString(fichier);
	jeu->designers.nElements = lireUint8(fichier);
	// Rendu ici, les champs précédents de la structure jeu sont remplis avec la bonne information.

	//TODO: Ajouter en mémoire le jeu lu. Il faut revoyer le pointeur créé.
	// Attention, il faut aussi créer un tableau dynamique pour les designers
	// que contient un jeu. Servez-vous de votre fonction d'ajout de jeu car la
	// liste de jeux participé est une ListeJeu. Afficher un message lorsque
	// l'allocation du jeu est réussie.
	int capacite = jeu->designers.nElements;
	Designer** nouvelleListeDesigner = new Designer * [capacite];
	jeu->designers.elements = nouvelleListeDesigner;
	
	//cout << jeu.titre << endl;  //TODO: Enlever cet affichage temporaire servant à voir que le code fourni lit bien les jeux.
	for ([[maybe_unused]] int i : iter::range(jeu->designers.nElements)) {
		Designer* designer = lireDesigner(fichier, listePrincipale);  //TODO: Mettre le designer dans la liste des designer du jeu.
		jeu->designers.elements[i] = designer;
		//TODO: Ajouter le jeu à la liste des jeux auquel a participé le designer.
		ajouterJeu(designer->listeJeuxParticipes, jeu);
	}
	return jeu; //TODO: Retourner le pointeur vers le nouveau jeu.
}

ListeJeux creerListeJeux(const string& nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);
	int nElements = lireUint16(fichier);
	ListeJeux listeJeux = {};
	for([[maybe_unused]] int n : iter::range(nElements))
	{
		ajouterJeu(listeJeux, lireJeu(fichier, listeJeux)); //TODO: Ajouter le jeu à la ListeJeux.
	}

	return listeJeux; //TODO: Renvoyer la ListeJeux.
}

//TODO: Fonction pour détruire un designer (libération de mémoire allouée).
// Lorsqu'on détruit un designer, on affiche son nom pour fins de débogage.
void detruireDesigner(Designer* designer)
{
	std::cout << "Destruction du designer: " << designer->nom << endl; //Affichage nom pour fins de débogage.
	delete[] designer->listeJeuxParticipes.elements;
	delete designer;
}

//TODO: Fonction qui détermine si un designer participe encore à un jeu.
bool designerParticipeEncoreJeu(Jeu* jeu, ListeJeux& liste)
{
	for (Designer* designer : spanListeDesigners(jeu->designers))
	{
		if (designer->listeJeuxParticipes.nElements == 1)
		{
			return true;
		}
		return false;	
	}
}

//TODO: Fonction pour détruire un jeu (libération de mémoire allouée).
// Attention, ici il faut relâcher toute les cases mémoires occupées par un jeu.
// Par conséquent, il va falloir gérer le cas des designers (un jeu contenant
// une ListeDesigners). On doit commencer par enlever le jeu à détruire des jeux
// qu'un designer a participé (listeJeuxParticipes). Si le designer n'a plus de
// jeux présents dans sa liste de jeux participés, il faut le supprimer.  Pour
// fins de débogage, affichez le nom du jeu lors de sa destruction.
void detruireJeu(Jeu* jeu, ListeJeux& liste)
{
	enleverJeu(liste, jeu);
	for (Designer* designer : spanListeDesigners(jeu->designers))
	{
		if (designerParticipeEncoreJeu(jeu, liste) == true)
		{
			detruireDesigner(designer);
		}
		else
		{
			int i = 0;
			while (designer->listeJeuxParticipes.elements[i] != jeu)
				++i;
			designer->listeJeuxParticipes.elements[i] = designer->listeJeuxParticipes.elements[designer->listeJeuxParticipes.nElements - 1];
			designer->listeJeuxParticipes.nElements--;
		}
	}
	cout << "Destruction du jeu: " << jeu->titre << endl; //Affichage nom du jeu pour fins de débogage.
	delete[] jeu->designers.elements;
	delete jeu;
}

//TODO: Fonction pour détruire une ListeJeux et tous ses jeux.
void detruireListeJeux(ListeJeux& liste)
{
	for (Jeu* jeu : spanListeJeux(liste))
	{
		detruireJeu(jeu, liste);
	}
	delete[] liste.elements;
}

void afficherDesigner(const Designer& d)
{
	cout << "\t" << d.nom << ", " << d.anneeNaissance << ", " << d.pays
			  << endl;
}

//TODO: Fonction pour afficher les infos d'un jeu ainsi que ses designers.
// Servez-vous de la fonction afficherDesigner ci-dessus.
void afficherJeu(const Jeu& jeu)
{
	cout << jeu.titre << endl;
	cout << jeu.anneeSortie << endl;
	cout << jeu.developpeur << endl;
	ListeDesigners listeDesigners = jeu.designers;
	for (Designer* designer : spanListeDesigners(listeDesigners))
	{
		afficherDesigner(*designer);
	}
}

//TODO: Fonction pour afficher tous les jeux de ListeJeux, séparés par un ligne.
// Servez-vous de la fonction d'affichage d'un jeu crée ci-dessus. Votre ligne
// de séparation doit être différent de celle utilisée dans le main.
void afficherListeJeux(const ListeJeux& listeJeux)
{
	static const string ligneDeSeparation = "═════════════════════════════════════════════════════════════════════════";
	cout << ligneDeSeparation << endl;
	for (const Jeu* jeu : spanListeJeux(listeJeux))
	{
		afficherJeu(*jeu);
		cout << ligneDeSeparation << endl;
	}
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
	#pragma region "Bibliothèque du cours"
	// Permet sous Windows les "ANSI escape code" pour changer de couleur
	// https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac
	// les supportent normalement par défaut.
	bibliotheque_cours::activerCouleursAnsi(); 
	#pragma endregion

	//int* fuite = new int;  // Pour vérifier que la détection de fuites fonctionne; un message devrait dire qu'il y a une fuite à cette ligne.

	ListeJeux listeJeux = creerListeJeux("jeux.bin"); //TODO: Appeler correctement votre fonction de création de la liste de jeux.

	static const string ligneSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";
	cout << ligneSeparation << endl;
	cout << "Premier jeu de la liste :" << endl;
	//TODO: Afficher le premier jeu de la liste (en utilisant la fonction).  Devrait être Chrono Trigger.
	afficherJeu(*listeJeux.elements[0]);

	cout << ligneSeparation << endl;

	//TODO: Appel à votre fonction d'affichage de votre liste de jeux.
	cout << "La liste de jeux est :" << endl;
	afficherListeJeux(listeJeux);

	cout << ligneSeparation << endl;

	//TODO: Faire les appels à toutes vos fonctions/méthodes pour voir qu'elles fonctionnent et avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.

	//TODO: Détruire tout avant de terminer le programme.  Devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
	detruireListeJeux(listeJeux);
}
