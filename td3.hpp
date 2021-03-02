#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <memory>
#include "gsl/span"
using gsl::span;
using namespace std;


struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
	ListeFilms() = default;
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements_; }
	void detruire(bool possedeLesFilms = false);

private:
	int capacite_ = 0, nElements_ = 0;
	Film** elements_ = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
};

template <typename T>
class Liste {
public:
	Liste() = default;
	Liste(int nElements) {
		nElements_ = capacite_ = nElements;
		elements_ = make_unique<shared_ptr<T>[]>(nElements_);
	}
	Liste& operator=(const Liste& autre) { 
		
		if (this != &autre)
		{
			elements_ = make_unique<shared_ptr<T>[]>(autre.capacite_);
			for (int i = 0; i < autre.nElements_; ++i)
			{
				elements_[i] = autre.elements_[i];
			}
			capacite_ = autre.capacite_;
			nElements_ = autre.nElements_;
		}
		return *this; 
	}
	Liste(const Liste& autre) {
		elements_ = make_unique<shared_ptr<T>[]>(autre.capacite_);
		for (int i =0; i < autre.nElements_; ++i)
		{
			elements_[i] = autre.elements_[i];
		}
		capacite_ = autre.capacite_;
		nElements_ = autre.nElements_;
	}
	span<shared_ptr<T>> enSpan() const {
		auto elements = elements_.get();
		return span(elements, nElements_);
	}
	/*span<Acteur*> enSpan() const {
		auto elements = elements_.get();
		return span(elements, nElements_);
	}*/
	//int size() const { return nElements_; }
	unique_ptr<shared_ptr<T>[]> getElements() { return elements_; }
	//unique_ptr<shared_ptr<T>[]> getCapacite() { return capacite_; }
	//void setElements(unique_ptr<shared_ptr<T>[]> nouveauElements) { elements_ = nouveauElements; }
	//void setnElements(int nouveaunElements) { nElements_ = nouveaunElements; }
	//void setCapacite(int nouvelleCapacite) { capacite_ = nouvelleCapacite; }
	void setElement(shared_ptr<T> nouveauElement, int pos) { elements_[pos] = nouveauElement; }

private:
	int capacite_ = 0, nElements_ = 0;
	//Acteur** elements_;
	//unique_ptr<Acteur*[]> elements_;
	unique_ptr<shared_ptr<T>[]> elements_; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.
};

using ListeActeurs = Liste<Acteur>;

struct Film
{
	string titre = "", realisateur = ""; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	int anneeSortie = 0, recette = 0; // Année de sortie et recette globale du film en millions de dollars
	ListeActeurs acteurs;
};

struct Acteur
{
	string nom = ""; int anneeNaissance = 0; char sexe ='N';
	//ListeFilms joueDans;
};
