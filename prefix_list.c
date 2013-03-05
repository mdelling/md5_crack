#include "prefix_list.h"

char lowercase_prefixes[PREFIX_TABLE_SIZE][PREFIX_LENGTH] = {
			"den", "all", "pin", "lar", "kat", "nin", "str", "ana",
			"sol", "cho", "cas", "pla", "shi", "ger", "hel", "bob",
			"tit", "sim", "mai", "col", "ver", "pan", "lan", "bas",
			"lor", "bra", "lou", "wer", "ann", "con", "del", "ant",
			"val", "bal", "bel", "jul", "tra", "gol", "kal", "cor",
			"mag", "cal", "har", "pas", "mic", "kil", "leo", "can",
			"tri", "red", "pro", "lin", "mad", "her", "mer", "bla",
			"hal", "ali", "mac", "fra", "pop", "dra", "mis", "mik",
			"par", "sar", "gra", "big", "bou", "ser", "mel", "per",
			"pat", "pap", "bab", "flo", "mil", "and", "lov", "tim",
			"ber", "luc", "han", "mas", "sal", "che", "nic", "jan",
			"kin", "kar", "sta", "der", "ste", "lil", "mor", "fre",
			"tom", "bar", "sha", "max", "ale", "chi", "san", "dar",
			"ben", "mam", "mal", "pol", "dan", "sam", "mon", "the",
			"car", "min", "mat", "lol", "cha", "man", "sch", "mar" };

char numeric_prefixes[PREFIX_TABLE_SIZE][PREFIX_LENGTH] = {
			"234", "741", "002", "025", "666", "254", "213", "555",
			"987", "052", "124", "223", "224", "142", "152", "007",
			"016", "291", "225", "153", "145", "242", "202", "024",
			"246", "951", "232", "066", "312", "135", "023", "014",
			"252", "022", "258", "125", "147", "132", "161", "181",
			"789", "456", "017", "032", "196", "154", "111", "156",
			"321", "281", "155", "062", "212", "171", "071", "271",
			"000", "311", "031", "001", "122", "151", "211", "011",
			"041", "051", "301", "102", "091", "112", "261", "141",
			"131", "012", "081", "241", "231", "021", "201", "251",
			"221", "310", "159", "061", "197", "101", "121", "190",
			"290", "280", "160", "270", "300", "170", "070", "040",
			"180", "080", "240", "260", "050", "150", "110", "090",
			"140", "030", "230", "130", "060", "220", "020", "250",
			"120", "100", "210", "010", "198", "200", "123", "199" };

char camel_prefixes[PREFIX_TABLE_SIZE][PREFIX_LENGTH] = {
			"Mar", "Sch", "Man", "Ben", "Luc", "Jan", "Dan", "Max",
			"Tim", "Ste", "Tom", "Mat", "Han", "Flo", "Nic", "Sam",
			"Mon", "Ale", "And", "Sha", "Bla", "Mal", "Jul", "Car",
			"Leo", "Lin", "San", "Sim", "Sta", "Ali", "Dar", "Har",
			"Min", "Ber", "Pol", "Cha", "Fre", "Mic", "Fra", "Her",
			"Bar", "Mil", "Jen", "Rob", "Chi", "Mag", "Mik", "Len",
			"Mel", "Lil", "Mam", "Pet", "Ann", "Tri", "Mor", "Ang",
			"Nik", "Jon", "Jac", "Lol", "Lau", "Sal", "Hei", "The",
			"Tig", "Ram", "Tra", "Bra", "Str", "Lar", "Mer", "Pan",
			"Lor", "Gra", "Cor", "Ant", "Gre", "Par", "Bob", "Chr",
			"Ser", "Sar", "Jes", "Big", "Spi", "Pat", "Mir", "Nat",
			"Che", "Mad", "Wil", "Ger", "Pla", "Ric", "Cas", "Ron",
			"Eli", "Lan", "Jos", "Tho", "Per", "Pro", "Bab", "Nor",
			"Der", "Gar", "Joh", "Kar", "Wer", "Coo", "Val", "Mis",
			"Con", "Tor", "Sto", "Die", "Bil", "Kev", "Col", "Tro" };
