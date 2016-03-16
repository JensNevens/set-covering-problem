#!/bin/sh

# $1 is path of scp executable
# $2 is path of folder with instances
# $3 is path of folder to save results

seed=123456789

#Remove old files
for f in "$3"/*.txt
do
    rm ${f}
done

#Exercise 1.1
for a in "ch1" "ch2" "ch3" "ch4"; do
    for re in "" "--re"; do
        start=$SECONDS
        if [ -z $re ] # if empty string
        then
            configuration="$a"
        else
            configuration="$a+re"
        fi
        for instance in "$2"/*
        do
            y=${instance%.txt} # get part before .txt
            cost=$(eval "$1 --seed $seed --instance $instance --$a $re")
            filename=${y##*/} # Only keep filename (remove the path)
            length=${#filename}
            first=$(echo ${filename:3:$length-4}  | awk '{print toupper($0)}')
            second=${filename:$length-1:1}
            echo "$first.$second $cost" >> "$3/$configuration.txt"
        done
        duration=$(( SECONDS - start ))
        echo "$configuration $duration" >> "$3/ex1_time.txt"
    done
done


##Exercise 1.2
for a in "ch1" "ch4"; do
    for re in "" "--re"; do
        for imp_algo in "fi" "bi"; do
            start=$SECONDS
            if [ -z $re ] # if empty string
            then
                configuration="$a+$imp_algo"
            else
                configuration="$a+re+$imp_algo"
            fi
            for instance in "$2"/*
            do
                y=${instance%.txt} # get part before .txt
                cost=$(eval "$1 --seed $seed --instance $instance --$a $re --$imp_algo")
                filename=${y##*/} # Only keep filename (remove the path)
                length=${#filename}
                first=$(echo ${filename:3:$length-4}  | awk '{print toupper($0)}')
                second=${filename:$length-1:1}
                echo "$first.$second $cost" >> "$3/$configuration.txt"
            done
            duration=$(( SECONDS - start ))
            echo "$configuration $duration" >> "$3/ex2_time.txt"
        done
    done
done

