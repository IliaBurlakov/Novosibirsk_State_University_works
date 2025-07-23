package calculator;

import calculator.commands.*;
import calculator.exceptions.CalculatorException;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.logging.*;

public class Calculator {
    private static final Logger logger = Logger.getLogger(Calculator.class.getName());

    public static void main(String[] args) {
        setupLogger();
        logger.info("Calculator started");

        Context context = new Context();
        CommandFactory factory = new CommandFactory();

        try (BufferedReader reader = getReader(args)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.trim().isEmpty() || line.trim().startsWith("#"))
                    continue;

                String[] partsOfLine = line.trim().split("\\s+");
                Command command = factory.getCommand(partsOfLine);

                if (command == null) {
                    logger.warning("Unknown command: " + partsOfLine[0]);
                    continue;
                }

                try {
                    command.execute(context, partsOfLine);
                    logger.info("Executed command: " + partsOfLine[0] + " " + String.join(" ", Arrays.copyOfRange(partsOfLine, 1, partsOfLine.length)) + ", " + context);
                } catch (CalculatorException e) {
                    logger.log(Level.WARNING, "Error while executing command: " + e.getMessage(), e);
                }
            }
        } catch (Exception e) {
            logger.log(Level.SEVERE, "Error while reading commands: " + e.getMessage(), e);
        }

        logger.info("Calculator finished");
    }

    private static BufferedReader getReader(String[] args) throws IllegalArgumentException {
        if (args.length == 0) {
            return new BufferedReader(new InputStreamReader(System.in));
        } else if (args.length == 1) {
            try {
                return new BufferedReader(new FileReader(args[0]));
            } catch (FileNotFoundException e) {
                logger.log(Level.SEVERE, "File \"" + args[0] + "\" was not found.", e);
                System.exit(1);
                return null;
            }
        } else {
            throw new IllegalArgumentException("Expected 0 or 1 argument, got: " + args.length);
        }
    }

    private static void setupLogger() {
        try {
            Logger rootLogger = Logger.getLogger("");
            for (Handler handler : rootLogger.getHandlers()) {
                rootLogger.removeHandler(handler);
            }
            FileHandler fileHandler = new FileHandler("calculator.log", false);
            fileHandler.setLevel(Level.INFO);
            fileHandler.setFormatter(new SimpleFormatter());
            rootLogger.addHandler(fileHandler);
            rootLogger.setLevel(Level.INFO);
        } catch (Exception e) {
            System.err.println("Error setting up logger: " + e.getMessage());
        }
    }
}
