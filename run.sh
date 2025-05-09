#!/bin/zsh
start_time=$EPOCHSECONDS

# Launch processes in the background and store their PIDs
python3 utils/renderVoronoiPointsSegments.py -i outputs/ps0.txt -o outputs/ps0.png --dpi 480 -v & pids+=($!)
python3 utils/renderVoronoiPointsSegments.py -i outputs/ps1.txt -o outputs/ps1.png --dpi 480 -v & pids+=($!)
python3 utils/renderVoronoiPointsSegments.py -i outputs/ps2.txt -o outputs/ps2.png --dpi 480 -v & pids+=($!)
python3 utils/renderObjectArray.py -i outputs/m0.txt -o outputs/m0.png --dpi 480 -v & pids+=($!)
python3 utils/renderObjectArray.py -i outputs/m1.txt -o outputs/m1.png --dpi 480 -v & pids+=($!)
python3 utils/renderObjectArray.py -i outputs/m2.txt -o outputs/m2.png --dpi 480 -v & pids+=($!)

# Wait for all and print individual completions
for pid in $pids; do
    wait $pid
    echo "Process $pid completed."
done

end_time=$EPOCHSECONDS
elapsed=$((end_time - start_time))

echo "All processes completed."
echo "Total execution time: ${elapsed} seconds"