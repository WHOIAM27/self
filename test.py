print('===== Continuous Python Calculator =====')

# Start the indefinite loop
while True:
    # 1. Get user input
    num1 = float(input('\nEnter first number: '))
    op = input('Enter operator (+, -, *, /): ')
    num2 = float(input('Enter second number: '))
    
    # 2. Perform the calculation
    if op == '+':
        result = num1 + num2
        print(f'{num1} + {num2} = {result}')
    elif op == '-':
        result = num1 - num2
        print(f'{num1} - {num2} = {result}')
    elif op == '*':
        result = num1 * num2
        print(f'{num1} * {num2} = {result}')
    elif op == '/':
        if num2 == 0:
            print('Error: Division by zero is not allowed!')
        else:
            result = num1 / num2
            print(f'{num1} / {num2} = {result}')
    else:
        print(f"'{op}' is an invalid operator. Please use +, -, *, or /")
    
    # 3. Check if the user wants to continue
    choice = input('\nDo you want to perform another calculation? (yes/no): ').strip().lower()
    
    # If they type anything other than 'yes', we break the loop
    if choice != 'yes' and choice != 'y':
        print('Calculator closed. Goodbye!')
        break  # This keyword instantly exits the while loop