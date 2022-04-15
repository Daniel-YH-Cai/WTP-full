with open('WTP-base/testcase.txt', 'w') as f:
    for i in range(97,123):
        content=chr(i)*1024+"\n"
        f.write(content)
    for i in range(48,58):
        content=chr(i)*1024+"\n"
        f.write(content)
