
# Launch processes in the background and store their PIDs

# python3 utils/renderVoronoiPointsSegments.py -i outputs/ps0.txt -o outputs/ps0.png --dpi 480 -v & pids+=($!)
# python3 utils/renderVoronoiPointsSegments.py -i outputs/ps1.txt -o outputs/ps1.png --dpi 480 -v & pids+=($!)
# python3 utils/renderVoronoiPointsSegments.py -i outputs/ps2.txt -o outputs/ps2.png --dpi 480 -v & pids+=($!)

# python3 utils/renderObjectArray.py -i outputs/m0.txt -o outputs/m0.png --pinSize 3.5 --dpi 480 -v &
# python3 utils/renderObjectArray.py -i outputs/m1.txt -o outputs/m1.png --pinSize 3.5 --dpi 480 -v &
# python3 utils/renderObjectArray.py -i outputs/m2.txt -o outputs/m2.png --pinSize 3.5 --dpi 480 -v & 

# python3 utils/renderVoronoiGraph.py -i outputs/vd0.txt -o outputs/vd0.png --dpi 480 -v & pids+=($!)
# python3 utils/renderVoronoiGraph.py -i outputs/vd1.txt -o outputs/vd1.png --dpi 480 -v & pids+=($!)
# python3 utils/renderVoronoiGraph.py -i outputs/vd2.txt -o outputs/vd2.png --dpi 480 -v & pids+=($!)

# python3 utils/renderVoronoiPolygon.py -i outputs/mp0.txt -o outputs/mp0.png --dpi 480 -v & pids+=($!)
# python3 utils/renderVoronoiPolygon.py -i outputs/mp1.txt -o outputs/mp1.png --dpi 480 -v & pids+=($!)
# python3 utils/renderVoronoiPolygon.py -i outputs/mp2.txt -o outputs/mp2.png --dpi 480 -v & pids+=($!)


python3 utils/renderDiffusionEngine.py -i outputs/dsem0.txt --dpi 1200 -v -o outputs/dsem0.png & 
python3 utils/renderDiffusionEngine.py -i outputs/dsem1.txt --dpi 1200 -v -o outputs/dsem1.png &
python3 utils/renderDiffusionEngine.py -i outputs/dsem2.txt --dpi 1200 -v -o outputs/dsem2.png &