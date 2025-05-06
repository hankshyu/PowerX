pinWidth = 122
pinHeight = 122

leftborderwidth = -1


if pinHeight > 3:
    print(f"Cord(0, 0) to Cord({pinWidth - 1}, 0)")
    print(f"Cord(0, 1) to Cord({pinWidth - 1}, 1)")
    print(f"Cord(0, 2) to Cord({pinWidth - 1}, 2)")

j = 3
alternating = False
while j <= (pinHeight - 1):
    if alternating == True:
        if pinWidth >= leftborderwidth:
            print(f"Cord(0, {j}) to Cord({leftborderwidth - 1}, {j})")
        else:
            print(f"Cord(0, {j}) to Cord({pinWidth - 1}, {j})")

        i = leftborderwidth
    else:
        if pinWidth >= (leftborderwidth + 3):
            print(f"Cord(0, {j}) to Cord({leftborderwidth + 2}, {j})")
        else:
            print(f"Cord(0, {j}) to Cord({pinWidth - 1}, {j})")
        i = leftborderwidth + 2
    
    while (i+1) <= (pinWidth - 1):
        if((i+6) <= (pinWidth - 1)):
            print(f"Cord({i+1}, {j}) to Cord({i+5}, {j})")
        else:
            print(f"Cord({i+1}, {j}) to Cord({pinWidth - 1}, {j})")
        i = i + 6

    for k in range(1, 6):
        if((j+k) <= (pinHeight-1)):
            print(f"Cord(0, {j + k}) to Cord({pinWidth - 1}, {j + k})")
    j = j + 6
    alternating = not alternating
