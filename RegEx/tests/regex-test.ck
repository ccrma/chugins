<<< RegEx.match( ".txt", "hey.txt" ) >>>;
<<< RegEx.match( ".txt", "hey.cpp" ) >>>;

string matches[0];
<<< RegEx.match( ".txt", "hey.txt", matches ) >>>;

for( int i; i < matches.size(); i++ )
{
    <<< "match:", matches[i] >>>;
}