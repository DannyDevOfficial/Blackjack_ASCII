//============================================================================
// Name        : Blackjack.cpp
// Author      : Daniel Grieco
// Version     :
// Copyright   : All Rights Reserved. Owned by Daniel Grieco ©
// Description : Blackjack in C++, Ansi-style
//============================================================================

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cctype>
#include <cstring>

using namespace std;

enum class Score {
	DEALER_MIN = 17,
	BLACKJACK = 21
};

// "Hard" hand is when the player doesn't have an ace valued 11,
// "Soft" is the opposite.
enum class HandType {
	HARD,
	SOFT
};

// Types of players in the game.
enum class PlayerType {
	PLAYER,
	DEALER
};

// All the possible face cards.
enum class FaceCard {
	NONE,
	JACK,
	QUEEN,
	KING
};

// All the card seeds.
enum class Seed {
	NONE,
	CLUBS,
	DIAMONDS,
	HEARTS,
	SPADES
};

// The card data.
struct Card {
	int id;
	int value;
	Seed seed;
	FaceCard face;
};

// The player/dealer data.
struct Participant {
	static const int HAND_CARDS_TOT = 9;

	PlayerType type;
	Card cards[HAND_CARDS_TOT];
	int nextHandCardsIndex;
	int totalScore;
	HandType hand;
	bool isDone;
	bool hasBusted;
};

// The deck data.
struct Deck {
	static const int CARD_TOT = 52;

	Card cards[CARD_TOT];
	int drawnCardsIDs[CARD_TOT];
	int nextDrawnCardsIndex;
};

/// FUNCTIONS

void InitParticipant(PlayerType type, Participant& p);
void MakeDeck(Deck& deck);
Card* MakeCard(int theValue, FaceCard theFace, Seed theSeed);
void InitCard(Card& card);
void ShuffleDeck(Deck& deck, int shuffles);
void GiveHand(Participant& p, Deck& d);
bool IsCardAlreadyDrawn(const Card& drawnCard, const Deck& deck);
void UpdateScore(Participant& p);
void DisplayStats(const Participant& p);
char* ReadCard(const Card& cardToRead);
char* ReadHand(const HandType handToRead);
void DrawCard(Participant& prtcpnt, Deck& dck);
void UpdateDrawCards(Participant& partcpt, Deck& deck, const Card& crd);
bool WantToGetHit();
void CheckAndDisplayWinner(const Participant& plyr, const Participant& dlr);
bool WantToPlayAgain();
bool GetBoolean(const char question[]);

int main() {
	// Seed the random number generator.
	srand(time(nullptr));

	// Deck of cards to use.
	Deck theDeck;
	// Participants.
	Participant dealer;
	Participant player;

	do {
		InitParticipant(PlayerType::DEALER, dealer);
		InitParticipant(PlayerType::PLAYER, player);

		MakeDeck(theDeck);
		ShuffleDeck(theDeck, 40);

		GiveHand(player, theDeck);
		GiveHand(dealer, theDeck);

		// Update the score.
		UpdateScore(player);
		UpdateScore(dealer);

		// Display player's hand as long as they aren't done.
		DisplayStats(player);

		// Hit the player until they say they're fine or they bust.
		if (!player.isDone) {
			while (true) {
				// Ask the player if they're fine with their hand.
				// If we get here then the player hasn't busted nor are they done.
				player.isDone = !WantToGetHit();

				// Player is done, no more hits.
				if (player.isDone)
					break;

				// Give the player another card.
				DrawCard(player, theDeck);

				// Update the score.
				UpdateScore(player);

				// Display player's hand.
				DisplayStats(player);

				// Player has busted or hit a blackjack.
				if (player.isDone || player.hasBusted)
					break;
			}
		}

		// Player is done or has busted.
		// Hit the dealer now, if necessary.
		if (!player.hasBusted) {
			while (true) {
				// Dealer minimum score has to be at least 17.
				dealer.isDone = (dealer.totalScore >= ((int) Score::DEALER_MIN)) && (dealer.hand == HandType::HARD);

				// Dealer is done, no more hits.
				if (dealer.isDone)
					break;

				// Give the dealer another card.
				DrawCard(dealer, theDeck);

				// Update the score.
				UpdateScore(dealer);

				// Dealer has busted.
				if (dealer.hasBusted)
					break;
			}
		}

		// Display info for the dealer at the hand.
		DisplayStats(dealer);

		// Check for the winner and display it.
		CheckAndDisplayWinner(player, dealer);
	} while (WantToPlayAgain());

	return 0;
}

// Initializes a participant.
void InitParticipant(PlayerType type, Participant& p) {
	p.type = type;

	for (int i = 0; i < p.HAND_CARDS_TOT; i++)
		InitCard(p.cards[i]);

	p.nextHandCardsIndex = 0;
	p.totalScore = 0;
	p.hand = HandType::HARD;
	p.isDone = false;
	p.hasBusted = false;
}

// Creates a deck of cards in order.
void MakeDeck(Deck& deck) {
	// Initialize all cards in the deck.
	for (int i = 0; i < deck.CARD_TOT; i++)
		InitCard(deck.cards[i]);

	// Initialize drawn cards index.
	deck.nextDrawnCardsIndex = 0;

	Card* op_currentCard = nullptr;

	int val = 0;
	FaceCard f = FaceCard::NONE;
	Seed s = Seed::CLUBS;
	// Add all 52 cards.
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 13; j++) {
			// Values from 10 and above are all worth 10 points.
			if ((j + 1) >= 10) {
				val = 10;

				// Assign the right face card.
				switch (j + 1) {
					case 11:
						f = FaceCard::JACK;
						break;
					case 12:
						f = FaceCard::QUEEN;
						break;
					case 13:
						f = FaceCard::KING;
						break;
				}
			} else {
				// Normal values coming before 10.
				val = j + 1;
				// None of these card is a face card, because it's less than 10.
				f = FaceCard::NONE;
			}

			// Create new card.
			op_currentCard = MakeCard(val, f, s);

			// Card is good, so we add it to the deck.
			if (op_currentCard) {
				deck.cards[j + (i * 13)] = *op_currentCard;

				// Free card memory.
				delete op_currentCard;
				op_currentCard = nullptr;
			}
		}

		// Next seed.
		switch (i + 1) {
			case 1:
				s = Seed::DIAMONDS;
				break;
			case 2:
				s = Seed::HEARTS;
				break;
			case 3:
				s = Seed::SPADES;
				break;
		}
	}

	// There are no cards drawn at this moment, so initialize that to 0.
	for (int i = 0; i < deck.CARD_TOT; i++)
		deck.drawnCardsIDs[i] = 0;
}

// Initializes a card to its default value.
void InitCard(Card& card) {
	card.id = 0;
	card.value = 0;
	card.face = FaceCard::NONE;
	card.seed = Seed::NONE;
}

// Creates a card based on the given arguments and returns it.
Card* MakeCard(int theValue, FaceCard theFace, Seed theSeed) {
	Card* np_newCard = new Card;

	// Value.
	np_newCard->value = theValue;

	// Face.
	np_newCard->face = theFace;

	// Used to generate ID.
	int idFaceVal = -1;
	switch (theFace) {
		case FaceCard::JACK:
			idFaceVal = 0;
			break;
		case FaceCard::QUEEN:
			idFaceVal = 1;
			break;
		case FaceCard::KING:
			idFaceVal = 2;
			break;
		case FaceCard::NONE:
			idFaceVal = 3;
			break;
	}

	// Seed.
	np_newCard->seed = theSeed;

	// Used to generate ID.
	int idSeedVal = -1;
	switch (theSeed) {
		case Seed::CLUBS:
			idSeedVal = 0;
			break;
		case Seed::DIAMONDS:
			idSeedVal = 1;
			break;
		case Seed::HEARTS:
			idSeedVal = 2;
			break;
		case Seed::SPADES:
			idSeedVal = 3;
			break;
		case Seed::NONE:
			idSeedVal = -1;
			break;
	}

	// Generate a unique ID
	np_newCard->id =  ((np_newCard->value + 100) * (idFaceVal + 1)) + idSeedVal;

	return np_newCard;
}

// Shuffles the given deck the given amount of times.
void ShuffleDeck(Deck& deck, int shuffles) {
	// Shuffling the deck a few times.
	for (int i = 0; i < shuffles; i++) {
		// Indices used for random cards to swap.
		int randCard1Index = rand() % deck.CARD_TOT;
		int randCard2Index = 0;

		// We don't want to have the two indices to be the same,
		// hence, we regenerate an index if they're equal.
		do {
			randCard2Index = rand() % deck.CARD_TOT;
		} while (randCard1Index == randCard2Index);

		// Card swapping.
		Card temp = deck.cards[randCard1Index];
		deck.cards[randCard1Index] = deck.cards[randCard2Index];
		deck.cards[randCard2Index] = temp;
	}
}

// Creates the hand for the participant.
void GiveHand(Participant& p, Deck& d) {
	int randCardIndex = 0;
	Card card;

	// Two loops for two cards.
	for (int i = 0; i < 2; i++) {
		// Draw a card until it's a new card.
		do {
			randCardIndex = rand() % d.CARD_TOT;
			card = d.cards[randCardIndex];
		} while (IsCardAlreadyDrawn(card, d));

		// Card is new.
		UpdateDrawCards(p, d, card);
	}
}

// Checks if a card has already been drawn or not and returns
// the result.
bool IsCardAlreadyDrawn(const Card& drawnCard, const Deck& deck) {
	// Loop as many times as the number of cards already drawn.
	for (int i = 0; i < deck.nextDrawnCardsIndex; i++) {
		// When a matching ID is found, the given card has already been drawn.
		if (drawnCard.id == deck.drawnCardsIDs[i])
			return true;
	}

	// If we get here then the card hasn't been drawn yet.
	return false;
}

// Updates the participant's score.
void UpdateScore(Participant& p) {
	int score = 0;
	bool hasAce = false;

	// Loop through all the cards and add up the score.
	for (int i = 0; i < p.nextHandCardsIndex; i++) {
		// Ace count increase.
		if (p.cards[i].value == 1)
			hasAce = true;

		// Add up to the score normally.
		score += p.cards[i].value;
	}

	// No ace means normal score.
	if (!hasAce) {
		p.totalScore = score;
		p.hand = HandType::HARD;
	} else {
		// One ace can be valued eleven or one based on
		// the situation.

		// If the score is greater than the blackjack when we add 11 to it,
		// then the ace is valued one, otherwise is valued 11.
		if ((score + 10) > ((int) Score::BLACKJACK)) {
			p.totalScore = score;
			p.hand = HandType::HARD;
		} else {
			p.totalScore = score + 10;
			p.hand = HandType::SOFT;
		}
	}

	// When the score is 21, participant is done.
	// When the score is greater than 21, participant has busted.
	if (p.totalScore > ((int) Score::BLACKJACK))		p.hasBusted = true;
	else if (p.totalScore == ((int) Score::BLACKJACK))	p.isDone = true;
}

// Display info about a participant.
void DisplayStats(const Participant& p) {
	// Print some text about the participant type.
	switch (p.type) {
		case PlayerType::PLAYER:
			cout << "Your info: " << endl;
			break;
		case PlayerType::DEALER:
			cout << "Dealer's info: " << endl;
			break;
	}

	// Style.
	cout << "*************************" << endl;

	char* op_cardStats = nullptr;
	for (int i = 0; i < p.nextHandCardsIndex; i++) {
		// Print card info.
		op_cardStats = ReadCard(p.cards[i]);
		cout << op_cardStats << endl;

		// Free up memory from the previous card.
		delete[] op_cardStats;
		op_cardStats = nullptr;
	}

	// Spacing.
	cout << endl;

	// Print score.
	cout << "Current score: " << p.totalScore << endl;

	// Spacing.
	cout << endl;

	// Print hand.
	char* op_handStats = ReadHand(p.hand);
	cout << op_handStats << endl;

	// Free up memory from the hand info.
	delete[] op_handStats;
	op_handStats = nullptr;

	// Style.
	cout << "*************************" << endl;

	// Spacing.
	cout << endl;
}

// Returns a string with info about the given card.
char* ReadCard(const Card& cardToRead) {
	char* np_cardInfo = new char[20];

	// Value string.
	const char* valueStr = to_string(cardToRead.value).c_str();

	// Pick the string based on face if the value is a 10.
	if (cardToRead.value == 10) {
		// With face cards we don't want to print the value.
		if (cardToRead.face == FaceCard::NONE) {
			// Add the value to the card info.
			strcpy(np_cardInfo, valueStr);
		} else {
			switch (cardToRead.face) {
				case FaceCard::NONE:
					break;
				case FaceCard::JACK:
					strcpy(np_cardInfo, "Jack");
					break;
				case FaceCard::QUEEN:
					strcpy(np_cardInfo, "Queen");
					break;
				case FaceCard::KING:
					strcpy(np_cardInfo, "King");
					break;
			}
		}
	} else if (cardToRead.value == 1) {
		// Replace 1 with Ace.
		strcpy(np_cardInfo, "Ace");
	} else {
		// Add the value to the card info.
		strcpy(np_cardInfo, valueStr);
	}

	// Add text to the string.
	strcat(np_cardInfo, " of ");

	// Pick the string based on the seed.
	switch (cardToRead.seed) {
		case Seed::NONE:
			break;
		case Seed::CLUBS:
			strcat(np_cardInfo, "Clubs.");
			break;
		case Seed::DIAMONDS:
			strcat(np_cardInfo, "Diamonds.");
			break;
		case Seed::HEARTS:
			strcat(np_cardInfo, "Hearts.");
			break;
		case Seed::SPADES:
			strcat(np_cardInfo, "Spades.");
			break;
	}

	return np_cardInfo;
}

// Returns a string with info about the given hand type.
char* ReadHand(const HandType handToRead) {
	char* np_handInfo = new char[22];

	// Add text to the string.
	strcpy(np_handInfo, "You have a ");

	// Pick the string based on the hand type.
	switch (handToRead) {
		case HandType::HARD:
			strcat(np_handInfo, "hard");
			break;
		case HandType::SOFT:
			strcat(np_handInfo, "soft");
			break;
	}

	// Add text to the string.
	strcat(np_handInfo, " hand.");

	return np_handInfo;
}

// Draws a random card.
void DrawCard(Participant& prtcpnt, Deck& dck) {
	int randCardIndex = 0;
	Card card;

	// Draw a card until it's a new card.
	do {
		randCardIndex = rand() % dck.CARD_TOT;
		card = dck.cards[randCardIndex];
	} while (IsCardAlreadyDrawn(card, dck));

	// Card is new.
	UpdateDrawCards(prtcpnt, dck, card);
}

// Updates the drawn cards.
void UpdateDrawCards(Participant& partcpt, Deck& deck, const Card& crd) {
	// Add it to the participant's hand.
	partcpt.cards[partcpt.nextHandCardsIndex] = crd;
	partcpt.nextHandCardsIndex++;
	// Store the drawn card's ID, so we remember which cards have been drawn.
	deck.drawnCardsIDs[deck.nextDrawnCardsIndex] = crd.id;
	deck.nextDrawnCardsIndex++;
}

// Returns true if the player wants a hit, false otherwise.
bool WantToGetHit() {
	char q[] = "Hit? (y/n) ";
	return GetBoolean(q);
}

// Checks the scores and displays the winner based on that.
void CheckAndDisplayWinner(const Participant& plyr, const Participant& dlr) {
	int plyrScore = plyr.totalScore;
	int dlrScore = dlr.totalScore;

	// Check for anyone who has busted.
	if (plyr.hasBusted) {
		cout << "Player has busted! The dealer won!" << endl;
	} else if (dlr.hasBusted) {
		cout << "Dealer has busted! The player won!" << endl;
	} else if (plyrScore == dlrScore) {
		// Push game.
		if (plyrScore == ((int) Score::BLACKJACK))
			cout << "Player and dealer both hit a blackjack!" << endl;
		else
			cout << "Player score is " << plyrScore << ". Dealer score is " << dlrScore << endl;

		cout << "It's a push!" << endl;
	} else if (plyrScore > dlrScore) {
		// Player wins.
		if (plyrScore == ((int) Score::BLACKJACK))
			cout << "Player hit the blackjack! Player won!" << endl;
		else
			cout << "Player score is " << plyrScore << ". Player won!" << endl;
	} else {
		// Dealer wins.
		if (dlrScore == ((int) Score::BLACKJACK))
			cout << "Dealer hit the blackjack! Dealer won!" << endl;
		else
			cout << "Dealer score is " << dlrScore << ". Dealer won!" << endl;
	}

	// Spacing.
	cout << endl;
}

// Returns true if the user wants to play again, false otherwise.
bool WantToPlayAgain() {
	char q[] = "Want to play again? (y/n) ";
	return GetBoolean(q);
}

// Returns a boolean value based on the answer to the given question.
bool GetBoolean(const char question[]) {
	const int IGNORE_CHARS = 256;
	bool failure = false;

	// Answer.
	char a = ' ';
	bool isTrue = false;

	do {
		// Reset the flag.
		failure = false;

		cout << question;
		cin >> a;

		// Turn the answer into lowercase.
		a = tolower(a);

		if (cin.fail()) {
			// Clear buffer.
			cin.clear();
			cin.ignore(IGNORE_CHARS, '\n');

			cout << "Wrong input type! Try again." << endl;
			cout << endl;

			failure = true;
		} else if ((a != 'y') && (a != 'n')) {
			// Ignore the character.
			cin.ignore(IGNORE_CHARS, '\n');

			cout << "Incorrect character! Try again." << endl;
			cout << endl;

			failure = true;
		} else {
			// Set the isTrue flag based on the answer.
			switch (a) {
				case 'y':
					isTrue = true;
					break;
				case 'n':
					isTrue = false;
					break;
			}

			failure = false;
		}
	} while (failure);

	// Spacing.
	cout << endl;

	return isTrue;
}
