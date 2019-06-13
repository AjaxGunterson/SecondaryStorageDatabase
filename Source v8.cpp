#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
using namespace std;

/*Constants*/

const char DELIMITER = '*';

void printMenu(); //Print interactive menu
int getInput();      //Get integer from user input
bool isInt(const std::string s); //Check if the passed string is a valid non negative integer
bool find_value(int key, string file, int recordsPerBlock, int blockSize, int initialBlocks);
bool delete_value(int key, string file, int recordsPerBlock, int blockSize, int initialBlocks);
bool insert_value(string input, string file, int recordsPerBlock, int blockSize, int initialBlocks);
vector<string> split(string splitted, char delim);
int getNumOfBlocks(string file, int recordsPerBlock, int blockSize);
int stringToInt(string input);
void printSequenceSet(string file);

int RECORDS_PER_BLOCK = 4,
	BLOCK_SIZE = RECORDS_PER_BLOCK * 100;	//Calculate max block size based on records per block.

int main(int argc, char* argv[])
{
	string	inputFile,
			outputFile;
	char	option;
	bool	verbose = false;
	//Parse the command line
	for (int i = 1; i < argc; i++)
	{
		option = *argv[i];

		switch (option)
		{
		case 'i':
			inputFile = argv[++i];
			break;
		case 'o':
			outputFile = argv[++i];
			break;
		case 'r':
			RECORDS_PER_BLOCK = stoi(argv[++i]);
			break;
		case 'v':
			verbose = true;
			break;
		default:
			cout << argv[i] << " command not recognized/n";
			break;
		}
	}




	//Set variables to command line arguments


	//Print command line arguments if verbose
	if (verbose)
	{
		cout << "Input file is " << inputFile << endl;
		cout << "Output file is " << outputFile << endl;
		cout << "Block size is " << BLOCK_SIZE << endl;
		cout << "Records per block is " << RECORDS_PER_BLOCK << endl;
	}

    
    struct Record {
        int zip;
        string place,
            state,
            county,
            latitude,
            longitude;
    };

    struct Block {
        int pred_block,
            succ_block,
            number_inside;
        Record * records;
        
        Block(int rpb)
        {
            records = new Record[rpb];
        }

        ~Block()
        {
            delete[] records;
        }
    };

    vector<string> recordBuffer;
    string buffer;
	int block_id = 0,
		search_key,
		initialBlocks;
    bool present;

    /*Input File*/
    ifstream myfile(inputFile.c_str());
    if (!myfile) {
        cerr << "Error opening input file..." << endl;
        exit(1);
    }
    /*Output File*/
    ofstream outfile(outputFile.c_str());
    if (!outfile) {
        cerr << "Error opening output tree..." << endl;
        exit(2);
    }



    while (myfile)//stop at eof
    {
        int recordsRead = 0;
        Block* blocky = new Block(RECORDS_PER_BLOCK);
        blocky->number_inside = 0;
        blocky->pred_block = block_id;
        blocky->succ_block = ++block_id;
        for (int i = 0; (i < RECORDS_PER_BLOCK) && getline(myfile, buffer); i++)
        {
            blocky->number_inside += 1;
            //getline(myfile, buffer);//get entire record
            recordBuffer = split(buffer, '\t');//split record into parts

                                               /*Enter split record into struct*/
                                               /*Assumes that a record will always have 6 total fields,
                                               even if empty*/
            blocky->records[i].zip = stringToInt(recordBuffer[0]);
            blocky->records[i].place = recordBuffer[1];
            blocky->records[i].state = recordBuffer[2];
            blocky->records[i].county = recordBuffer[3];
            blocky->records[i].longitude = recordBuffer[4];
            blocky->records[i].latitude = recordBuffer[5];
            recordsRead++;
        }//end for
		if (recordsRead > 0) {//make sure to not produce empty block
			outfile.seekp(BLOCK_SIZE * blocky->pred_block);
			outfile << blocky->pred_block << "*";
			outfile << blocky->succ_block << "*";
		}
        for (int i = 0; i < RECORDS_PER_BLOCK && i < recordsRead; i++)
        {
            /*Places struct parts into tree file*/
            outfile << blocky->records[i].zip << "*"
                << blocky->records[i].place << "*"
                << blocky->records[i].state << "*"
                << blocky->records[i].county << "*"
                << blocky->records[i].longitude << "*"
                << blocky->records[i].latitude << '\n';
        }//end for

        delete blocky;
    }//end while
    myfile.close();
    outfile.close();

	initialBlocks = getNumOfBlocks(outputFile, RECORDS_PER_BLOCK, BLOCK_SIZE);

    bool quit = false;
    bool validMenuOption = true;
    int menuItem;

    printMenu();
    while (quit == false) //Interactive menu quit condition 
    {
        std::cout << "Enter operation: ";

        do
        {
            menuItem = getInput();        //Get operation
            validMenuOption = true;        //Assume option was true
            int searchKey;
            bool deleted;
			string input,
				buffer;
			vector<string> recordBuffer;
            switch (menuItem)
            {
            case 1:
                cout << "Enter value you want find\n" << endl;
                cin >> searchKey;
                if (find_value(searchKey, outputFile, RECORDS_PER_BLOCK, BLOCK_SIZE, initialBlocks))
                {
                    cout << "Record found\n";
                } else {
                    cout << "Record not found\n";
                }
                break;
            case 2:
				/*Block for */
				cout << "Enter the new record:\n";
				cout << "Zip: ";
				cin >> buffer;
				input = buffer;
				cout << "Place: ";
				cin.ignore();
				getline(cin, buffer);
				input += "*" + buffer;
				cout << "State Code: ";
				cin >> buffer;
				input += "*" + buffer;
				cout << "County: ";
				cin.ignore();
				getline(cin, buffer);
				input += "*" + buffer;
				cout << "Longitude: ";
				cin >> buffer;
				input += "*" + buffer;
				cout << "Latitude: ";
				cin >> buffer;
				input += "*" + buffer;

				recordBuffer = split(input, '*');//split record into parts;

				if (find_value(stringToInt(recordBuffer[0]), outputFile, RECORDS_PER_BLOCK, BLOCK_SIZE, initialBlocks)) {
					cout << "Zip Code already present" << endl;
				}
				else {
					insert_value(input, outputFile, RECORDS_PER_BLOCK, BLOCK_SIZE, initialBlocks);
					cout << "\nValue Inserted\n\n"
						;
				}
                break;
            case 3:
                cout << "Enter the value you want to delete:" << endl;
                cin >> searchKey;
                deleted = delete_value(searchKey, outputFile, RECORDS_PER_BLOCK, BLOCK_SIZE, initialBlocks);
                if (deleted == true)
                    cout<<"\nRecord deleted\n";
                else
                    cout<<"\nRecord was not present in the file.\n";
                break;
            case 4:
                printSequenceSet(outputFile);
                break;
            case 0:
                quit = true;
                break;

            default:
                std::cout << "Invalid input! Try again: ";
                validMenuOption = false;
                break;
            }
        } while (validMenuOption == false);
    }
    return 0;
}

int getInput()
{
    std::string input;
    std::cin >> input;

    while (!isInt(input)) //Validate that input string is valid integer
    {
        std::cout << "Invalid input! Try again: ";
        std::cin >> input;
    }

    return std::atoi(input.c_str());
}

bool isInt(const std::string s)
{
    bool isInt = true;        //Holds if input string is int

                            //Check that all charachters of sting are digits
    for (int i = 0; i < s.length(); i++)
    {
        if (!isdigit(s[i]))
        {
            isInt = false;
            break;
        }
    }

    return isInt;
}

void printMenu()
{
    std::cout
        << "\nOperations Menu\n"
        << "1 Find record\n"
        << "2 Add record\n"
        << "3 Delete record\n"
        << "4 Print the sequence set\n"
        << "0 Quit\n"
        << "\n";
}
/*Prints the entire sequence set sequentially*/
void printSequenceSet(string file)
{
	int length;
	string buffer;
	vector<string> recordBuffer;
	ifstream myfile(file.c_str());

	if (!myfile)
	{
		cerr << "Error opening input tree..." << endl;
		exit(3);
	}
	cout << "\n";

	while (getline(myfile, buffer)) {
		recordBuffer = split(buffer, '*');//split record into parts
		if (recordBuffer.size() > 6) {
			if (recordBuffer[0].size() > recordBuffer[1].size()) {
				recordBuffer[0] = stringToInt(recordBuffer[1]) - 1;
			}
			if ((stringToInt(recordBuffer[2]) > 0)) {
				cout << recordBuffer[2] << '\t'
					<< recordBuffer[3] << '\t'
					<< recordBuffer[4] << '\t'
					<< recordBuffer[5] << '\t'
					<< recordBuffer[6] << '\t'
					<< recordBuffer[7] << '\n';
			}
		}//end if
		else if (stringToInt(recordBuffer[0]) > 0) {
			cout << recordBuffer[0] << '\t'
				<< recordBuffer[1] << '\t'
				<< recordBuffer[2] << '\t'
				<< recordBuffer[3] << '\t'
				<< recordBuffer[4] << '\t'
				<< recordBuffer[5] << '\n';
		}//end else
	}//end while
}

bool find_value(int key, string file, int RECORDS_PER_BLOCK, int BLOCK_SIZE, int initialBlocks) {
	{
		int prev,
			post,
			currentVal;
		string buffer;
		vector<string> recordBuffer;

		/*Input File*/
		fstream myfile(file.c_str());
		if (!myfile)
		{
			cerr << "Error i/o tree..." << endl;
			exit(3);
		}

		while (getline(myfile, buffer)) {

			for (int i = 0; i < RECORDS_PER_BLOCK; i++) {
				recordBuffer = split(buffer, '*');//split record into parts
				if (!(recordBuffer.size() < 6)) {//checks record for valid length
					if (recordBuffer.size() > 6) {//if first line of block
						prev = stringToInt(recordBuffer[0]);//previous block rrn
						post = stringToInt(recordBuffer[1]);//next block rrn
						currentVal = stringToInt(recordBuffer[2]);
						if (currentVal > key) {//had to disable to get new
							myfile.seekg(initialBlocks * BLOCK_SIZE);										//insert working
							while (getline(myfile, buffer)) {
								for (int i = 0; i < RECORDS_PER_BLOCK; i++) {
									recordBuffer = split(buffer, '*');//split record into parts
									if (!(recordBuffer.size() < 6)) {//checks record for valid length
										if (recordBuffer.size() > 6) {//if first line of block
											prev = stringToInt(recordBuffer[0]);//previous block rrn
											post = stringToInt(recordBuffer[1]);//next block rrn
											currentVal = stringToInt(recordBuffer[2]);

											if (currentVal == key)//if key is found
											{
												cout << '\n' << recordBuffer[2] << '\t'//zip
													<< recordBuffer[3] << '\t'//place
													<< recordBuffer[4] << '\t'//state
													<< recordBuffer[5] << '\t'//county
													<< recordBuffer[6] << '\t'//longitude
													<< recordBuffer[7] << '\t'//latitude
													<< endl;
												return true;
											}//end if
										}//end if
										else {//isn't first line of block
											currentVal = stringToInt(recordBuffer[0]);
											if (currentVal == key)//if key is found
											{
												cout << '\n' << recordBuffer[0] << '\t'//zip
													<< recordBuffer[1] << '\t'//place
													<< recordBuffer[2] << '\t'//state
													<< recordBuffer[3] << '\t'//county
													<< recordBuffer[4] << '\t'//longitude
													<< recordBuffer[5] << '\t'//latitude
													<< endl;
												return true;
											}//end if
										}//end else
									}//end if


									getline(myfile, buffer);
								}//end for
								myfile.seekg(post * BLOCK_SIZE);//go to next block
							}
						}

						if (currentVal == key)//if key is found
						{
							cout << '\n' << recordBuffer[2] << '\t'//zip
								<< recordBuffer[3] << '\t'//place
								<< recordBuffer[4] << '\t'//state
								<< recordBuffer[5] << '\t'//county
								<< recordBuffer[6] << '\t'//longitude
								<< recordBuffer[7] << '\t'//latitude
								<< endl;
							return true;
						}//end if
					}//end if
					else {//isn't first line of block
						currentVal = stringToInt(recordBuffer[0]);
						if (currentVal == key)//if key is found
						{
							cout << '\n' << recordBuffer[0] << '\t'//zip
								<< recordBuffer[1] << '\t'//place
								<< recordBuffer[2] << '\t'//state
								<< recordBuffer[3] << '\t'//county
								<< recordBuffer[4] << '\t'//longitude
								<< recordBuffer[5] << '\t'//latitude
								<< endl;
							return true;
						}//end if
					}//end else
				}//end if


				getline(myfile, buffer);
			}//end for
			myfile.seekg(post * BLOCK_SIZE);//go to next block
		}//end while

		return false;
	}
}

bool delete_value(int key, string file, int RECORDS_PER_BLOCK, int BLOCK_SIZE, int initialBlocks) {
	int prev,
		post;
	string buffer;
	vector<string> recordBuffer;
	int currentPos;

	/*Input File*/
	fstream myfile(file.c_str());
	if (!myfile)
	{
		cerr << "Error opening tree for i/o..." << endl;
		exit(3);
	}

	while (getline(myfile, buffer)) {

		for (int i = 0; i < RECORDS_PER_BLOCK; i++) {
			recordBuffer = split(buffer, '*');//split record into parts
			if (!(recordBuffer.size() < 6)) {//checks record for valid length
				if (recordBuffer.size() > 6) {//if first line of block
					prev = stringToInt(recordBuffer[0]);//previous block rrn
					post = stringToInt(recordBuffer[1]);//next block rrn
					if (recordBuffer[0].size() > recordBuffer[1].size()) {
						recordBuffer[0] = stringToInt(recordBuffer[1]) - 1;
					}
					if (stringToInt(recordBuffer[2]) == key)//if key is found
					{
						myfile.seekg(BLOCK_SIZE * prev + (recordBuffer[0].size() + recordBuffer[1].size() + 2));//goes to beginning of and then
																												//offset added to get to zip
						myfile.write("-1", recordBuffer[2].size());
						return true;
					}//end if
				}//end if
				else {//isn't first line of block
					if (stringToInt(recordBuffer[0]) == key)//if key is found
					{
						currentPos = myfile.tellp();//wouldn't let me use tellp in an expression
						myfile.seekg(currentPos - buffer.length() - 2);//aparrently newline takes up 2 spots so that's why
						myfile.write("-1", recordBuffer[0].size());
						return true;
					}//end if
				}//end else
			}


			getline(myfile, buffer);
		}//end for
		myfile.seekg(post * BLOCK_SIZE);//go to next block
	}//end while

	return false;
}

/*Inserts value*/
bool insert_value(string input, string file, int RECORDS_PER_BLOCK, int BLOCK_SIZE, int initialBlocks) {

	int numOfReads = 0,
		nextBlock = initialBlocks;
	string buffer;
	vector<string> inputRecord,
		recordBuffer;

	fstream myfile(file.c_str());
	if (!myfile)
	{
		cerr << "Error opening input tree..." << endl;
		exit(3);
	}

	inputRecord = split(input, '*');//split record into parts;
	myfile.seekg(nextBlock * BLOCK_SIZE);//go new block

	while (getline(myfile, buffer)) {

		for (int i = 0; i < RECORDS_PER_BLOCK; i++) {
			recordBuffer = split(buffer, '*');//split record into parts
			if (recordBuffer.size() < 6) {//checks to see if starting new block
				myfile << inputRecord[0] << "*"
					<< inputRecord[1] << "*"
					<< inputRecord[2] << "*"
					<< inputRecord[3] << "*"
					<< inputRecord[4] << "*"
					<< inputRecord[5] << '\n';
				return true;
			}
			else if (recordBuffer.size() == 6) {//do nothing
			}
			else {
				nextBlock = stringToInt(recordBuffer[1]);
			}

			getline(myfile, buffer);
			if (myfile.tellp() == -1) {//if eof is reached
				myfile.clear();
				myfile.seekg(nextBlock * BLOCK_SIZE);

				myfile << nextBlock << "*"
					<< nextBlock + 1 << "*"
					<< inputRecord[0] << "*"
					<< inputRecord[1] << "*"
					<< inputRecord[2] << "*"
					<< inputRecord[3] << "*"
					<< inputRecord[4] << "*"
					<< inputRecord[5] << '\n';
				return true;
			}

		}//end for
		myfile.seekg(nextBlock * BLOCK_SIZE);//go to next block

	}

	return false;
}

/*Seperates string into parts*/
vector<string> split(string toSplit, char delim) {

    stringstream ss(toSplit);
    string item;
    vector<string> splitString;
    while (getline(ss, item, delim)) {
        splitString.push_back(item);
    }

    return splitString;
}

/*Returns current number of blocks in the file*/
int getNumOfBlocks(string file, int RECORDS_PER_BLOCK, int BLOCK_SIZE){
	int blockNum = 0,
		nextBlock = 0;
	string buffer;
	vector<string> recordBuffer;

	/*Input File*/
	ifstream myfile(file.c_str());
	if (!myfile)
	{
		cerr << "Error opening input tree..." << endl;
		exit(3);
	}

	while (getline(myfile, buffer)) {//get last block number (until we have header)

		recordBuffer = split(buffer, '*');//split record into parts
		if (recordBuffer.size() > 6) {//if first line of block
			blockNum = stringToInt(recordBuffer[0]);//previous block rrn
			nextBlock = stringToInt(recordBuffer[1]);
		}//end if

		myfile.seekg(nextBlock * BLOCK_SIZE);//go to next block
	}

	return blockNum;
}

/*Converts string to integer without stoi*/
int stringToInt(string input) {
	int stringSum = 0;

	for (int i = 1; i <= input.length(); i++) {
		if (input[input.length() - i] <= 57 && input[input.length() - i] >= 48) {
			stringSum += (input[input.length() - i] - 48) * pow(10, i - 1);
		}
		else if (input[input.length() - i] == '-') {//checks for negative
			stringSum *= -1;
		}
	}
	return stringSum;
}