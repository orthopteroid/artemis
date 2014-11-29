package com.tereslogica.arcanacard;

import java.util.ArrayList;
import java.util.Random;

/**
 * Created by john on 27/11/14.
 */
public class FakeUserInput {
    // http://en.wikiquote.org/wiki/Jules_Verne
    private final static String shakespeare[] = {
            "Hamlet", "To be not to be that is the question", "Act III Scene I",
            "*", "Neither a borrower nor a lender be; For loan oft loses both itself and friend, and borrowing dulls the edge of husbandry", "Act I Scene III",
            "*", "This above all: to thine own self be true", "Act I Scene III",
            "*", "Though this be madness, yet there is method in 't.", "Act II Scene II",
            "*", "That it should come to this!", "Act I Scene II",
            "*", "There is nothing either good or bad, but thinking makes it so", "Act II Scene II",
            "*", "What a piece of work is man! How noble in reason! How infinite in faculty! In form and moving how express and admirable!", "Act II Scene II",
            "*", "In action how like an angel! In apprehension how like a god! The beauty of the world, the paragon of animals! ", "Act II Scene II",
            "*", "The lady doth protest too much, methinks", "Act III Scene II",
            "*", "In my mind's eye", "Act I Scene II",
            "*", "A little more than kin, and less than kind", "Act I Scene II",
            "*", "The play 's the thing wherein I'll catch the conscience of the king", "Act II Scene II",
            "*", "And it must follow, as the night the day, thou canst not then be false to any man", "Act I Scene III",
            "*", "This is the very ecstasy of love", "Act II Scene I",
            "*", "Brevity is the soul of wit", "Act II Scene II",
            "*", "Doubt that the sun doth move, doubt truth to be a liar, but never doubt I love", "Act II Scene II",
            "*", "Rich gifts wax poor when givers prove unkind", "Act III Scene I",
            "*", "Do you think I am easier to be played on than a pipe?", "Act III Scene II",
            "*", "I will speak daggers to her, but use none", "Act III Scene II",
            "*", "When sorrows come, they come not single spies, but in battalions", "Act IV Scene V",
            "As You Like It", "All the world's a stage, and all the men and women merely players.", "Act II Scene VII",
            "*", "They have their exits and their entrances; And one man in his time plays many parts", "Act II Scene VII",
            "*", "Can one desire too much of a good thing?", "Act IV Scene I",
            "*", "I like this place and willingly could waste my time in it", "Act II Scene IV",
            "*", "How bitter a thing it is to look into happiness through another man's eyes!", "Act V Scene II",
            "*", "Blow, blow, thou winter wind! Thou art not so unkind as man's ingratitude", "Act II Scene VII",
            "*", "True is it that we have seen better days", "Act II Scene VII",
            "*", "For ever and a day", "Act IV Scene I",
            "*", "The fool doth think he is wise, but the wise man knows himself to be a fool", "Act V Scene I",
            "King Richard III", "Now is the winter of our discontent", "Act I Scene I",
            "*", "A horse! a horse! my kingdom for a horse!", "Act V Scene IV",
            "*", "Conscience is but a word that cowards use, devised at first to keep the strong in awe", "Act V Scene III",
            "*", "So wise so young, they say, do never live long", "Act III Scene I",
            "*", "Off with his head!", "Act III Scene IV",
            "*", "An honest tale speeds best, being plainly told", "Act IV Scene IV",
            "*", "The king's name is a tower of strength", "Act V Scene III",
            "*", "The world is grown so bad, that wrens make prey where eagles dare not perch", "Act I Scene III",
            "Romeo and Juliet", "O Romeo, Romeo! wherefore art thou Romeo?", "Act II Scene II",
            "*", "It is the east, and Juliet is the sun", "Act II Scene II",
            "*", "Good Night, Good night! Parting is such sweet sorrow, that I shall say good night till it be morrow.", "Act II Scene II",
            "*", "What's in a name? That which we call a rose by any other name would smell as sweet", "Act II Scene II",
            "*", "Wisely and slow; they stumble that run fast", "Act II Scene III",
            "*", "Tempt not a desperate man", "Act V Scene III",
            "*", "For you and I are past our dancing days", "Act I Scene V",
            "*", "O! she doth teach the torches to burn bright", "Act I Scene V",
            "*", "It seems she hangs upon the cheek of night like a rich jewel in an Ethiope's ear", "Act I Scene V",
            "*", "See, how she leans her cheek upon her hand! O that I were a glove upon that hand, that I might touch that cheek!", "Act II Scene II",
            "*", "Not stepping o'er the bounds of modesty", "Act IV Scene II",
            "Merchant of Venice", "But love is blind, and lovers cannot see", "Act II Scene VII",
            "*", "If you prick us, do we not bleed? if you tickle us, do we not laugh? if you poison us, do we not die? and if you wrong us, shall we not revenge?", "Act III Scene I",
            "*", "The devil can cite Scripture for his purpose", "Act I Scene III",
            "*", "I like not fair terms and a villain's mind", "Act I Scene III",
            "Merry Wives of Windsor", "Why, then the world 's mine oyster", "Act II Scene II",
            "*", "This is the short and the long of it", "Act II Scene II",
            "*", "I cannot tell what the dickens his name is", "Act III Scene II",
            "*", "As good luck would have it", "Act III Scene V",
            "Measure for Measure", "Our doubts are traitors, and make us lose the good we oft might win, by fearing to attempt", "Act I Scene IV",
            "*", "Some rise by sin, and some by virtue fall", "Act II Scene I",
            "*", "The miserable have no other medicine but only hope", "Act III Scene I",
            "King Henry IV", "He will give the devil his due", "Act I Scene II",
            "*", "The better part of valour is discretion", "Act V Scene IV",
            "*", "He hath eaten me out of house and home", "Act II Scene I",
            "*", "Uneasy lies the head that wears a crown", "Act III Scene I",
            "*", "A man can die but once", "Act III Scene II",
            "*", "I do now remember the poor creature, small beer", "Act II Scene II",
            "*", "We have heard the chimes at midnight", "Act III Scene II",
            "*", "The smallest worm will turn, being trodden on", "Act II Scene II",
            "*", "Suspicion always haunts the guilty mind; The thief doth fear each bush an officer", "Act V Scene VI",
            "*", "Delays have dangerous ends", "Act III Scene II",
            "*", "Of all base passions, fear is the most accursed", "Act V Scene II",
            "*", "The first thing we do, let's kill all the lawyers", "Act IV Scene II",
            "*", "Small things make base men proud", "Act IV Scene I",
            "*", "True nobility is exempt from fear", "Act IV Scene I",
            "*", "Having nothing, nothing can he lose", "Act III Scene III",
            "Taming of the Shrew", "I 'll not budge an inch", "Induction Scene I",
            "Timon of Athens", "We have seen better days", "Act IV Scene II",
            "Julius Caesar", "Friends, Romans, countrymen, lend me your ears; I come to bury Caesar, not to praise him", "Act III Scene II",
            "*", "But, for my own part, it was Greek to me", "Act I Scene II",
            "*", "A dish fit for the gods", "Act II Scene I",
            "*", "Cry Havoc, and let slip the dogs of war!", "Act III Scene I",
            "*", "Et tu, Brute!", "Act III Scene I",
            "*", "Men at some time are masters of their fates: The fault, dear Brutus, is not in our stars, but in ourselves, that we are underlings", "Act I Scene II",
            "*", "Not that I loved Caesar less, but that I loved Rome more", "Act III Scene II",
            "*", "Beware the ides of March", "Act I Scene II",
            "*", "This was the noblest Roman of them all", "Act V Scene V",
            "*", "When that the poor have cried, Caesar hath wept: Ambition should be made of sterner stuff", "Act III Scene II",
            "*", "Yond Cassius has a lean and hungry look; He thinks too much: such men are dangerous", "Act I Scene II",
            "*", "For Brutus is an honourable man; So are they all, all honourable men", "Act III Scene II",
            "*", "As he was valiant, I honor him; but, as he was ambitious, I slew him", "Act III Scene II",
            "*", "Cowards die many times before their deaths; The valiant never taste of death but once.", "Act II Scene II",
            "*", "Of all the wonders that I yet have heard, it seems to me most strange that men should fear; Seeing that death, a necessary end, will come when it will come", "Act II Scene II",
            "Macbeth", "There 's daggers in men's smiles", "Act II Scene III",
            "*", "what 's done is done", "Act III Scene II",
            "*", "I dare do all that may become a man; Who dares do more is none", "Act I Scene VII",
            "*", "Fair is foul, and foul is fair", "Act I Scene I",
            "*", "I bear a charmed life", "Act V Scene VIII",
            "*", "Yet do I fear thy nature; It is too full o' the milk of human kindness.", "Act I Scene V",
            "*", "Will all great Neptune's ocean wash this blood clean from my hand? No, this my hand will rather the multitudinous seas incarnadine, making the green one red", "Act II Scene II",
            "*", "Double, double toil and trouble; Fire burn, and cauldron bubble.", "Act IV Scene I",
            "*", "Out, damned spot! out, I say!", "Act V Scene I",
            "*", "All the perfumes of Arabia will not sweeten this little hand.", "Act V Scene I",
            "*", "When shall we three meet again in thunder, lightning, or in rain? When the hurlyburly 's done, When the battle 's lost and won", "Act I Scene I",
            "*", "If chance will have me king, why, chance may crown me", "Act I Scene III",
            "*", "Nothing in his life became him like the leaving it; he died as one that had been studied in his death to throw away the dearest thing he owed, as t'were a careless trifle", "Act I Scene IV",
            "*", "Look like the innocent flower, but be the serpent under 't.", "Act I Scene V",
            "*", "I have no spur to prick the sides of my intent, but only vaulting ambition, which o'erleaps itself, and falls on the other.", "Act I Scene VII",
            "*", "Is this a dagger which I see before me, The handle toward my hand?", "Act II Scene I",
            "*", "Out, out, brief candle! Life's but a walking shadow, a poor player that struts and frets his hour upon the stage and then is heard no more: it is a tale told by an idiot, full of sound and fury, signifying nothing.", "Act V Scene V",
            "King Lear", "How sharper than a serpent's tooth it is to have a thankless child!", "Act I Scene IV",
            "*", "I am a man more sinned against than sinning", "Act III Scene II",
            "*", "My love's more richer than my tongue", "Act I Scene I",
            "*", "Nothing will come of nothing.", "Act I Scene I",
            "*", "Have more than thou showest, speak less than thou knowest, lend less than thou owest", "Act I Scene IV",
            "*", "The worst is not, So long as we can say, 'This is the worst.' ", "Act IV Scene I",
            "Othello", "‘T’is neither here nor there.", "Act IV Scene III",
            "*", "I will wear my heart upon my sleeve for daws to peck at", "Act I Scene I",
            "*", "To mourn a mischief that is past and gone is the next way to draw new mischief on", "Act I Scene III",
            "*", "The robbed that smiles steals something from the thief", "Act I Scene III",
            "Antony and Cleopatra", "My salad days, when I was green in judgment.", "Act I Scene V",
            "Cymbeline", "The game is up.", "Act III Scene III",
            "*", "I have not slept one wink.", "Act III Scene III",
            "Twelfth Night", "Be not afraid of greatness: some are born great, some achieve greatness and some have greatness thrust upon them", "Act II Scene V",
            "*", "Love sought is good, but giv'n unsought is better", "Act III Scene I",
            "King Henry V", "Men of few words are the best men", "Act III Scene II",
            "Midsummer Night's Dream", "The course of true love never did run smooth", "Act I Scene I",
            "*", "Love looks not with the eyes, but with the mind, and therefore is winged Cupid painted blind", "Act I Scene I",
            "Much Ado About Nothing", "Everyone can master a grief but he that has it", "Act III Scene II",
            "Titus Andronicus", "These words are razors to my wounded heart", "Act I Scene I",
            "Winter's Tale", "What 's gone and what's past help should be past grief", "Act III Scene II",
            "*", "You pay a great deal too dear for what's given freely", "Act I Scene I",
            "Taming of the Shrew", "Out of the jaws of death", "Act III Scene IV",
            "*", "Thus the whirligig of time brings in his revenges", "Act V Scene I",
            "*", "For the rain it raineth every day", "Act V Scene I",
    };

    private final static String julesverneEN[] = {
            "Journey to the Center of the Earth", "The undulation of these infinite numbers of mountains, whose snowy summits make them look as if covered by foam, recalled to my remembrance the surface of a storm-beaten ocean.", "1864",
            "*", "If I looked towards the west, the ocean lay before me in all its majestic grandeur, a continuation as it were, of these fleecy hilltops.", "1864",
            "*", "I became intoxicated with a sense of lofty sublimity, without thought of the abysses into which my daring was soon about to plunge me.", "1864",
            "*", "But Heaven never sends unmixed grief, and for Professor Liedenbrock there was a satisfaction in store proportioned to his desperate anxieties.", "1864",
            "*", "How many prisoners in solitary confinement become idiots, if not mad, for want of exercise for the thinking faculty!", "1864",
            "*", "To describe my despair would be impossible. No words could tell it. I was buried alive, with the prospect before me of dying of hunger and thirst.", "1864",
            "*", "Science, my lad, has been built upon many errors; but they are errors which it was good to fall into, for they led to the truth.", "1864",
            "*", "The Great Architect of the universe built it of good stuff.", "1864",
            "*", "The brain is at work without its required food, and the most fantastic notions fill the mind. Hitherto I had never known what hunger really meant.", "1864",
            "*", "Man is so constituted that health is a purely negative state. Hunger once satisfied, it is difficult for a man to imagine the horrors of starvation; they cannot be understood without being felt.", "1864",
            "*", "While his heart still beats, while his flesh still moves, I cannot accept that a being endowed with will-power can give in to despair.", "1864",
            "Earth to the Moon", "Now when an American has an idea, he directly seeks a second American to share it.", "1865",
            "*", "In America, all is easy, all is simple; and as for mechanical difficulties, they are overcome before they arise.", "1865",
            "*", "The moon, by her comparative proximity, and the constantly varying appearances produced by her several phases, has always occupied a considerable share of the attention of the inhabitants of the earth.", "1865",
            "*", "Their remains but the third class, the superstitious. These worthies were not content merely to rest in ignorance; they must know all about things which had no existence whatever.", "1865",
            "*", "They did to others that which they would not they should do to them—that grand principle of immorality upon which rests the whole art of war.", "1865",
            "*", "In spite of the opinions of certain narrow-minded people, who would shut up the human race upon this globe, as within some magic circle which it must never outstep, we shall one day travel to the moon, the planets, and the stars, with the same facility, rapidity, and certainty as we now make the voyage from Liverpool to New York!", "1865",
            "Twenty Thousand Leagues Under the Sea", "The human mind delights in grand conceptions of supernatural beings. And the sea is precisely their best vehicle, the only medium through which these giants (against which terrestrial animals, such as elephants or rhinoceroses, are as nothing) can be produced or developed.", "1870",
            "*", "Whoever calls himself Canadian calls himself French.", "1870",
            "*", "This forcible abduction, so roughly carried out, was accomplished with the rapidity of lightning. I shivered all over. Whom had we to deal with? No doubt some new sort of pirates, who explored the sea in their own way. Hardly had the narrow panel closed upon me, when I was enveloped in darkness.", "1870",
            "*", "We were alone. Where, I could not say, hardly imagine. All was black, and such a dense black that, after some minutes, my eyes had not been able to discern even the faintest glimmer.", "1870",
            "*", "What good would it be to discuss such a proposition, when force could destroy the best arguments?", "1870",
            "*", "The sea is everything. It covers seven tenths of the terrestrial globe. Its breath is pure and healthy. It is an immense desert, where man is never lonely, for he feels life stirring on all sides. The sea is only the embodiment of a supernatural and wonderful existence. It is nothing but love and emotion.", "1870",
            "*", "In the memory of the dead all chronological differences are effaced.", "1870",
            "*", "The Earth does not want new continents, but new men.", "1870",
            "*", "The Nautilus was piercing the water with its sharp spur, after having accomplished nearly ten thousand leagues in three months and a half, a distance greater than the great circle of the earth. Where were we going now, and what was reserved for the future?", "1870",
            "*", "We cannot prevent equilibrium from producing its effects. We may brave human laws, but we cannot resist natural ones.", "1870",
            "*", "Thus ends the voyage under the seas. What passed during that night — how the boat escaped from the eddies of the maelstrom — how Ned Land, Conseil, and myself ever came out of the gulf, I cannot tell.", "1870",
            "Fur Country", "Hobson perceived with some alarm that bears were very numerous in the neighbourhood and that scarcely a day passed without one or more of them being sighted.", "1872",
            "Around the World in Eighty Days", "Everybody knows that the great reversed triangle of land, with its base in the north and its apex in the south, which is called India, embraces fourteen hundred thousand square miles, upon which is spread unequally a population of one hundred and eighty millions of souls.", "1873",
            "*", "It was all very well for an Englishman like [himself] to make [the tour] with a carpet-bag; a lady could not be expected to travel comfortably under such conditions.", "1873",
            "*", "He had won his wager, and had made his journey... To do this he had employed every means of conveyance — steamers, railways, carriages, yachts, trading-vessels, sledges, elephants.", "1873",
            "*", "Truly, would you not for less than that make the tour around the world?", "1873",
            "Mysterious Island", "Better to put things at the worst at first and reserve the best for a surprise.", "1874",
            "*", "Before all masters, necessity is the one most listened to, and who teaches the best.", "1874",
            "*", "An energetic man will succeed where an indolent one would vegetate and inevitably perish.", "1874",
            "*", "Man is never perfect, nor contented.", "1874",
            "*", "It is a great misfortune to be alone, my friends; and it must be believed that solitude can quickly destroy reason.", "1874",
            "*", "Men, however learned they may be, can never change anything of the cosmographical order established by God Himself.", "1874",
            "*", "...the world is very learned. What a big book, captain, might be made with all that is known!", "1874",
            "*", "And what a much bigger book still with all that is not known!", "1874",
            "*", "The desire to perform a work which will endure, which will survive him, is the origin of his superiority over all other living creatures here below.", "1874",
            "*", "Civilization never recedes; the law of necessity ever forces it onwards.", "1874",
            "*", "He who is mistaken in an action which he sincerely believes to be right may be an enemy, but retains our esteem.", "1874",
    };

    private final static String julesverneFR[] = {
            "Voyage au Centre de la Terre", "Les ondulations de ces montagnes infinies, que leurs couches de neige semblaient rendre écumantes, rappelaient à mon souvenir la surface d'une mer agitée.", "1864",
            "*", "Je me plongeais ainsi dans cette prestigieuse extase que donnent les hautes cimes, et cette fois, sans vertige, car je m'accoutumais enfin à ces sublimes contemplations.", "1864",
            "*", "Mes regards éblouis se baignaient dans la transparente irradiation des rayons solaires, j'oubliais qui j'étais, où j'étais, pour vivre de la vie des elfes ou des sylphes, imaginaires habitants de la mythologie scandinave; je m'enivrais de la volupté des hauteurs, sans songer aux abîmes dans lesquels ma destinée allait me plonger avant peu.", "1864",
            "*", "Mais aux grandes douleurs le ciel mêle incessamment les grandes joies, et il réservait au professeur Lidenbrock une satisfaction égale à ses désespérants ennuis.", "1864",
            "*", "Les objets extérieurs ont une action réelle sur le cerveau. Qui s’enferme entre quatre murs finit par perdre la faculté d’associer les idées et les mots. Que de prisonniers cellulaires devenus imbéciles, sinon fous, par le défaut d’exercice des facultés pensantes.", "1864",
            "*", "Je ne puis peindre mon désespoir ; nul mot de la langue humaine ne rendrait mes sentiments. J’étais enterré vif, avec la perspective de mourir dans les tortures de la faim et de la soif.", "1864",
            "*", "La science, mon garçon, est faite d’erreurs, mais d’erreurs qu’il est bon de commettre, car elles mènent peu à peu à la vérité.", "1864",
            "*", "Le grand architecte de l'univers l'a construite on bons matériaux.", "1864",
            "*", "L’homme est ainsi fait, que sa santé est un effet purement négatif; une fois le besoin de manger satisfait, on se figure difficilement les horreurs de la faim; il faut les éprouver, pour les comprendre.", "1864",
            "*", "Et tant que son coeur bat, tant que sa chair palpite, je n'admets pas qu'un être doué de volonté laisse en lui place au désespoir.", "1864",
            "De la Terre à la Lune", "Or, quand un Américain a une idée, il cherche un second Américain qui la partage.", "1865",
            "*", "Rien ne saurait étonner un Américain. On a souvent répété que le mot 'impossible' n’était pas français; on s’est évidemment trompé de dictionnaire.", "1865",
            "*", "L’astre des nuits, par sa proximité relative et le spectacle rapidement renouvelé de ses phases diverses, a tout d’abord partagé avec le Soleil l’attention des habitants de la Terre.", "1865",
            "*", "Restait en dernier lieu la classe superstitieuse des ignorants; ceux-lá ne se contentent pas d'ignorer, ils savent ce qui n'est pas.", "1865",
            "*", "Ils faisaient à autrui ce qu'ils ne voulaient pas qu'on leur fît, principe immoral sur lequel repose tout l’art de la guerre.", "1865",
            "*", "Il n'en est rien! On va aller à la Lune, on ira aux planètes, on ira aux étoiles, comme on va aujourd'hui de Liverpool à New York, facilement, rapidement, sûrement, et l'océan atmosphérique sera bientôt traversé comme les océans de la Lune!", "1865",
            "Vingt mille lieues sous les mers", "L'esprit humain se plaît à ces conceptions grandioses d'êtres surnaturels. Or la mer est précisément leur meilleur véhicule, le seul milieu où ces géants près desquels les animaux terrestres, éléphants ou rhinocéros, ne sont que des nains — puissent se produire et se développer.", "1870",
            "*", "Qui dit Canadien, dit Français.", "1865",
            "*", "Cet enlèvement, si brutalement exécuté, s'était accompli avec la rapidité de l'éclair... Un rapide frisson me glaça l'épiderme. A qui avions-nous affaire ? Sans doute à quelques pirates d'une nouvelle espèce qui exploitaient la mer à leur façon. A peine l'étroit panneau fut-il refermé sur moi, qu'une obscurité profonde m'enveloppa.", "1865",
            "*", "Nous étions seuls. Où ? Je ne pouvais le dire, à peine l'imaginer. Tout était noir, mais d'un noir si absolu, qu'après quelques minutes, mes yeux n'avaient encore pu saisir une de ces lueurs indéterminées qui flottent dans les plus profondes nuits.", "1865",
            "*", "A quoi bon discuter une proposition semblable, quand la force peut détruire les meilleurs arguments.", "1865",
            "*", "La mer est tout! Elle couvre... Son souffle est pur et sain. C'est l'immense désert où l'homme n'est jamais seul, car il sent frémir la vie à ses côtés. La mer n'est que le véhicule d'une surnaturelle et prodigieuse existence; elle n'est que mouvement et amour.", "1865",
            "*", "Les différences chronologiques s'effacent dans la mémoire des morts.", "1865",
            "*", "Ce ne sont pas de nouveaux continents qu'il faut à la terre, mais de nouveaux hommes!", "1865",
            "*", "Le Nautilus en brisait les eaux sous le tranchant de son éperon, après avoir accompli près de dix mille lieues en trois mois et demi, parcours supérieur à l'un des grands cercles de la terre. Où allions-nous maintenant, et que nous réservait l'avenir?", "1865",
            "*", "On ne saurait empêcher l'équilibre de produire ses effets. On peut braver les lois humaines, mais non résister aux lois naturelles.", "1865",
            "*", "Voici la conclusion de ce voyage sous les mers. Ce qui se passa pendant cette nuit, comment le canot échappa au formidable remous du Maelstrom, comment Ned Land, Conseil et moi, nous sortîmes du gouffre, je ne saurai le dire.", "1865",
            "Le pays des fourrures", "Hobson constata, non sans une certaine appréhension, que les ours étaient nombreux sur cette partie du territoire.", "1872",
            "Le Tour du monde en quatre-vingts jours", "Quant à voir la ville, il n'y pensait même pas, étant de cette race d'Anglais qui font visiter par leur domestique les pays qu'ils traversent.", "1873",
            "*", "Personne n'ignore que l'Inde — ce grand triangle renversé dont la base est au nord et la pointe au sud — comprend une superficie de quatorze cent mille milles carrés, sur laquelle est inégalement répandue une population de cent quatre-vingts millions d'habitants.", "1873",
            "*", "Qu'un Anglais comme lui fît le tour du monde un sac à la main, passe encore; mais une femme ne pouvait entreprendre une pareille traversée dans ces conditions.", "1873",
            "*", "Il avait employé pour ce faire tous les moyens de transport, paquebots, railways, voitures, yachts, bâtiments de commerce, traîneaux, éléphant.", "1873",
            "L’Île mystérieuse", "Mieux vaut mettre les choses au pis tout de suite, répondit l’ingénieur, et ne se réserver que la surprise du mieux.", "1874",
            "*", "La nécessité est, d’ailleurs, de tous les maîtres, celui qu’on écoute le plus et qui enseigne le mieux.", "1874",
            "*", "L’homme qui 'sait' réussit là où d’autres végéteraient et périraient inévitablement.", "1874",
            "*", "L’homme n’est jamais ni parfait, ni content.", "1874",
            "*", "Malheur à qui est seul, mes amis, et il faut croire que l’isolement a vite fait de détruire la raison.", "1874",
            "*", "Les hommes, si savants qu’ils puissent être, ne pourront jamais changer quoi que ce soit à l’ordre cosmographique établi par Dieu même.", "1874",
            "*", "Et pourtant, qui montra une certaine difficulté à se résigner, le monde est bien savant! Quel gros livre, monsieur Cyrus, on ferait avec tout ce qu’on sait!", "1874",
            "*", "Et quel plus gros livre encore avec tout ce qu’on ne sait pas!", "1874",
            "*", "Le besoin de faire œuvre qui dure, qui lui survive, est le signe de sa supériorité sur tout ce qui vit ici-bas.", "1874",
            "*", "La civilisation ne recule jamais, et il semble qu’elle emprunte tous les droits à la nécessité.", "1874",
            "*", "Celui qui se trompe dans une intention qu’il croit bonne, on peut le combattre, on ne cesse pas de l’estimer.", "1874",
    };

    private static String ClipRight( String s, int len ) {
        if( s.length() > len ) {
            return s.substring(0, len);
        } else {
            return s;
        }
    }

    private static String Ellipsis(String s, int len) {
        if( s.length() > len ) {
            int x = len/2 -3; // -3 for "..."
            return s.substring(0, x) + "..." + s.substring(s.length()-x, s.length());
        } else {
            return s;
        }
    }

    private static String[] SelectSubStrings( String s, int len, int count ) {
        Random r = new Random();

        // enum word starts
        ArrayList<Integer> wordpositions = new ArrayList<Integer>();
        int i = 0;
        while( i < s.length() ) {
            wordpositions.add(new Integer(i));
            while( ++i < s.length() && s.charAt(i) != ' ' ) { ; } // find next space
            ++i; // advance to next word-start
            if( i > s.length() - len/3 ) { break; } // skip words past end-tolerance
        }

        // add, but try to prevent duplicates
        ArrayList<String> arrl = new ArrayList<String>();
        for( int c=0; c<count; c++) {
            if( wordpositions.size() == 0 ) { // eek! ran out! recycle some of the others
                int recycle = r.nextInt(arrl.size());
                arrl.add( new String( arrl.get(recycle)));
            } else {
                // select a word, then remove it so it's not selected again
                int wordarrindex = r.nextInt(wordpositions.size());

                // remove *physically* adjacent words as well to prevent duplicates
                // use -1 for nonexistant cases
                int rightwordposition = wordarrindex +1 >= wordpositions.size() ? -1 : wordpositions.get(wordarrindex +1);
                int wordposition = wordpositions.get(wordarrindex);
                int leftwordposition = wordarrindex -1 <= 0 ? -1 : wordpositions.get(wordarrindex -1); // but not physical word 0

                int index = wordpositions.size();
                while( --index >= 0 ) {
                    if (wordpositions.get(index) == rightwordposition) { wordpositions.remove(index); }
                    else if (wordpositions.get(index) == wordposition) { wordpositions.remove(index); }
                    else if (wordpositions.get(index) == leftwordposition) { wordpositions.remove(index); break; }
                }

                // check length
                if( wordposition + len > s.length() ) {
                    arrl.add(s.substring(wordposition));
                } else {
                    arrl.add(s.substring(wordposition, wordposition + len));
                }
            }
        }
        return arrl.toArray(new String[arrl.size()]);
    }

    private static String[] SelectSubStrings1( String s, int len, int count ) {
        Random r = new Random();
        ArrayList<String> arrl = new ArrayList<String>();
        for( int c=0; c<count; c++) {
            int l = r.nextInt( s.length() - len );
            arrl.add( s.substring( l, l + len ) );
        }
        return arrl.toArray(new String[arrl.size()]);
    }

    private static String[] EnumCategories( String[] db ) {
        ArrayList<String> arrl = new ArrayList<String>();
        int i = 0;
        while( i < db.length ) {
            arrl.add( db[i] );
            do { i += 3; } while( i < db.length && db[i] == "*" ); // find next category
        }
        return arrl.toArray(new String[arrl.size()]);
    }

    private static class PassageClueSet {
        public String[] passages;
        public String[] clues;
        public PassageClueSet( String[] _p, String[] _c ) { passages=_p; clues=_c; }
    }

    private static PassageClueSet EnumPassages( String[] db, String cat ) {
        ArrayList<String> parr = new ArrayList<String>();
        ArrayList<String> carr = new ArrayList<String>();
        int i = 0;
        while( i < db.length && db[i] != cat ) { i+=3; }
        if( i < db.length ) {
            do {
                parr.add( db[++i] );
                carr.add( db[++i] );
                ++i;
            } while( i < db.length && db[i] == "*" ); // deal with all in same category
        }
        return new PassageClueSet( parr.toArray(new String[parr.size()]), (carr.toArray(new String[parr.size()]) ) );
    }

    //////////////////////

    public static class Scenario {
        public String m;
        public String l;
        public String[] k;
        public Scenario( String _m, String _l, String[] _k ) { m=_m; l=_l; k=_k; }

        public static Scenario MakeRndScenario( String[] db, int maxlen, int keycount ) {
            String lockClue = "";
            Random r = new Random();

            String[] catarr = EnumCategories( db );
            String cat = catarr[ r.nextInt( catarr.length ) ];
            PassageClueSet pcs = EnumPassages( db, cat );
            int passint = r.nextInt( pcs.passages.length );
            String pass = pcs.passages[ passint ];
            String clue = pcs.clues[ passint ];
            String[] keyclues = SelectSubStrings( pass, maxlen, keycount );
            return new Scenario( ClipRight(cat, maxlen), ClipRight(clue, maxlen), keyclues );
        }
    }

    public static Scenario Shakespeare( int maxlen, int keycount ) {
        return Scenario.MakeRndScenario( shakespeare, maxlen, keycount );
    }

    public static Scenario JulesVerneEN( int maxlen, int keycount ) {
        return Scenario.MakeRndScenario( julesverneEN, maxlen, keycount );
    }

    public static Scenario JulesVerneFR( int maxlen, int keycount ) {
        return Scenario.MakeRndScenario( julesverneFR, maxlen, keycount );
    }
};
