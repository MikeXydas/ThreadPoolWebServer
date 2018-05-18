#!/bin/bash

if [ $# -ne 4 ]; then
        echo "Not the correct number of arguments. Correct format is: ./webcreator.sh root_directory text_file w p"
        exit 1
fi

if [ ! -e $2 ]; then
        echo "Text file does not exist"
        exit 1
fi

if [ ! -d $1 ]; then
        echo "Root directory does not exist"
        exit 1
fi

if ! [[ $3 =~ ^[0-9]+$ ]]; then
        echo "Third argument (number of websites) must be a postive integer"
        exit 1
fi

if ! [[ $4 =~ ^[0-9]+$ ]]; then
        echo "Fourth argument (number of webpages) must be a positive integer"
        exit 1
fi

if ! [ -z "$(ls -A $1)" ]; then
        echo "Root_directory is not empty. Purging..."
        rm -rf $1
        mkdir $1
fi

lines=$(wc < $2 | awk -F ' ' ' END { print $1'})

if [ "$lines" -lt "10000" ]; then
        echo "Input text file does not have enough lines (< 10000)"
        exit 1
fi

#Creating the website directories
for whichWebsite in $(seq 0 $(($3 - 1)))
do
        mkdir $1/site$whichWebsite
done

for whichWebsite in $(seq 0 $(($3 - 1)))
do
        for whichPage in $(seq 0 $(($4 - 1)))
        do
                createdPflag=0
                while [ "$createdPflag" -eq "0" ]
                do
                        numb=$(( ( RANDOM % 10000 )  + 1 ))
                        numbExists=0
                        for  whichNumb in $(seq 0 $(($whichPage - 1)))
                        do
                                if [ $numb -eq ${numbs[$whichNumb]} ]; then
                                        numbExists=1
                                        break
                                fi
                        done

                        if [ "$numbExists" -eq "0" ]; then
                                numbs[$whichPage]=$numb
                                touch $1/site$whichWebsite/page${whichWebsite}_$numb.html
                                createdPflag=1
                        fi
                done
        done
        for whichPage in $(seq 0 $(($4 - 1)))
        do
                numbs[$whichNumb]=-1
        done
done

numbF=$(($4/2 + 1))
numbQ=$(($3/2 + 1))
#Initialisation of the external links
totalCombinations=$(( $3 * $4 ))

#Initialisation of the array we will use for checking incoming links
for i in $( seq 0 $(( $3 * $4 - 1 )) )
do
        linksCheck[$i]=0
done

#function that translates numbOfWebsite and numbOfPage on the specific webage
#findWebpage always returns on the $whichFile variable
whichFile="NULL"
findWebpage()
{
        whichFile=$(ls $1/site$2 | awk -v page=$3 'BEGIN {count=0} 
                                        {
                                                if(count == page)
                                                {
                                                        print $0
                                                        exit;
                                                }
                                                count++
                                        }
                                        ')
}


echo ""
for whichWebsite in $(seq 0 $(($3 - 1)))
do
        echo " Creating website $whichWebsite"
        for whichPage in $(seq 0 $(($4 - 1)))
        do
                findWebpage $1 $whichWebsite $whichPage
                k=$(( ( RANDOM % $(( $lines - 2000 )) )  + 1 ))
                m=$(( ( RANDOM % 999 )  + 1001 ))
                #m=20
                echo "  Creating page $1/site${whichWebsite}/$whichFile with $m lines, starting at line $k"

                #Randomly find unique internal links
                numbOfChoices=$(( $4 - 1))
                for choice in $(seq 0 $numbOfChoices)
                do
                        choices[$choice]=$choice
                done

                #Link on the same page not allowed (only exception when p==f)
                if [ "$4" -ne "$numbF" ]; then
                        choices[whichPage]=${choices[$numbOfChoices]}
                        numbOfChoices=$(( $numbOfChoices - 1 ))
                fi

                #Create a set of random internal links
                for intLink in $(seq 0 $(( $numbF - 1 )))
                do

                        numb=$(( ( RANDOM % $(( $numbOfChoices + 1 )) ) ))
                        internalSet[$intLink]=${choices[$numb]}
                        choices[$numb]=${choices[$numbOfChoices]}
                        numbOfChoices=$(( $numbOfChoices - 1 ))
                        linksCheck[$(( $whichWebsite * $4 + ${internalSet[$intLink]} ))]=1
                done
                
                #Randomly find unique external links
                for whichpage in $(seq 0 $(( $totalCombinations - 1 )) )
                do
                        currentSitePage[$whichpage]=$whichpage
                done

                #removing internal pages
                for i in $(seq 0 $(( $4 - 1 )) )
                do
                        currentSitePage[ $(( $whichWebsite * $4 + $i )) ]=${currentSitePage[ $(( $(( $3 - 1 )) * $4 + $i )) ]}
                done
                numbOfChoices=$(( $3 * $4 - $4 - 1))

                for extLink in $(seq 0 $(( $numbQ - 1 )))
                do
                        numb=$(( ( RANDOM % $(( $numbOfChoices + 1 )) ) ))
                        externalSet[$extLink]=${currentSitePage[$numb]}
                        currentSitePage[$numb]=${currentSitePage[$numbOfChoices]}
                        numbOfChoices=$(( $numbOfChoices - 1 ))
                        linksCheck[${externalSet[$extLink]}]=1
                done

                #findWebpage always returns on the $whichFile variable
                findWebpage $1 $whichWebsite $whichPage

                path=$1/site$whichWebsite/$whichFile

                #Write starting headers
                cat htmlHeaders/startingHeaders >> $path

                whichLine=$k
                whichF=0
                whichQ=0

                for whichLink in $( seq 0 $(( $numbF + $numbQ - 1 )) )
                do
                        #writing the text lines
                        for whichLineTemp in $( seq 0 $(( $m / ($numbF + $numbQ )  )) )
                        do
                                #sed -n "${whichLine}p" text | tr -d '\n' >> $path
                                lineCont=$(sed -n "${whichLine}p" text | tr -d '\n')
                                echo "        ${lineCont}<br>" >> $path
                                whichLine=$(( $whichLine + 1 ))
                        done

                        #randomly selecting a link
                        if [ "$whichF" -eq "$numbF" ]; then
                                chosenLink=1
                        elif [ "$whichQ" -eq "$numbQ" ]; then
                                chosenLink=0
                        else
                                chosenLink=$(( ( RANDOM % 2 ) ))
                        fi

                        #Internal link was chosen
                        if [ "$chosenLink" -eq "0" ]; then
                                findWebpage $1 $whichWebsite ${internalSet[$whichF]}
                                linkPath=../site$whichWebsite/$whichFile
                                echo "          Adding INTERNAL link to $1/site${whichWebsite}/$whichFile"
                                echo "        <a href="$linkPath">internal_link${whichLink}_text</a><br><br>" >> $path
                                whichF=$(( $whichF + 1 ))
                        else
                                tempWebsite=$(( externalSet[$whichQ] / $4 ))
                                tempPage=$(( externalSet[$whichQ] % $4 ))
                                findWebpage $1 $tempWebsite $tempPage
                                linkPath=../site${tempWebsite}/$whichFile
                                echo "          Adding EXTERNAL link to $1/site${tempWebsite}/$whichFile"
                                echo "        <a href="$linkPath">external_link${whichLink}_text</a><br><br>" >> $path
                                whichQ=$(( $whichQ + 1 ))
                        fi
                done

                #write the html ending
                cat htmlHeaders/endingHeaders >> $path


        done
        echo ""
done

incLinksFlag=1
for whichLink in $( seq 0 $(( $3 * $4 - 1 )) )
do
        if [ "${linksCheck[$whichLink]}" -eq "0" ]; then
                incLinksFlag=0
                break
        fi
done

if [ "$incLinksFlag" -eq "1" ]; then
        echo "All pages have at least one incoming link"
else
        echo "Some pages DO NOT have an incoming link"
fi

echo ""
echo "Done"
