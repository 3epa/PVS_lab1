package sim;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStreamReader;

public class SimulatorApp {
    private static final String DEFAULT_SIM_PORT = "/tmp/ttyV0";
    private static final String DEFAULT_USER_PORT = "/tmp/ttyV1";

    private static final String ANSI_RESET  = "\u001B[0m";
    private static final String ANSI_GRAY   = "\u001B[90m";
    private static final String ANSI_GREEN  = "\u001B[1;32m";
    private static final String ANSI_YELLOW = "\u001B[1;33m";
    private static final String ANSI_RED    = "\u001B[1;31m";

    public static void main(String[] args) throws Exception {
        Process socatProcess = null;
        String portPath;

        if (args.length > 0) {
            portPath = args[0];
        } else {
            System.out.println("Запуск socat для создания виртуальной пары COM-портов...");
            socatProcess = new ProcessBuilder(
                    "socat",
                    "pty,raw,echo=0,link=" + DEFAULT_SIM_PORT,
                    "pty,raw,echo=0,link=" + DEFAULT_USER_PORT
            ).redirectErrorStream(true).start();

            Runtime.getRuntime().addShutdownHook(new Thread(socatProcess::destroyForcibly));

            File simPortFile = new File(DEFAULT_SIM_PORT);
            for (int i = 0; i < 30 && !simPortFile.exists(); i++) {
                Thread.sleep(100);
            }
            if (!simPortFile.exists()) {
                System.err.println("Ошибка: не удалось создать виртуальный порт через socat.");
                System.err.println("Убедитесь, что socat установлен: sudo apt install socat");
                return;
            }
            portPath = DEFAULT_SIM_PORT;
        }

        System.out.println("==================================================================");
        System.out.println(" Симулятор стенда SDK-1.1M (Вариант 2: Гирлянда)");
        System.out.println(" Порт симулятора : " + portPath);
        if (socatProcess != null) {
            System.out.println(" Подключитесь из другого терминала командой:");
            System.out.println("   picocom -b 38400 " + DEFAULT_USER_PORT);
        }
        System.out.println(" Нажмите [Enter] в этом окне для имитации нажатия кнопки PC15");
        System.out.println("==================================================================\n");

        Garland garland = new Garland();

        try (FileInputStream in = new FileInputStream(portPath);
             FileOutputStream out = new FileOutputStream(portPath)) {

            final CliParser[] parserHolder = new CliParser[1];
            Runnable renderCallback = () -> renderStatus(garland, parserHolder[0]);
            CliParser cli = new CliParser(garland, out, renderCallback);
            parserHolder[0] = cli;

            renderStatus(garland, cli);

            Thread.ofVirtual().start(() -> {
                while (true) {
                    try {
                        if (garland.update()) {
                            renderStatus(garland, cli);
                        }
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        break;
                    }
                }
            });

            // Поток имитации физической кнопки PC15 по нажатию Enter в консоли симулятора
            Thread.ofVirtual().start(() -> {
                try (BufferedReader console = new BufferedReader(new InputStreamReader(System.in))) {
                    while (console.readLine() != null) {
                        garland.nextMode();
                        renderStatus(garland, cli);
                    }
                } catch (Exception ignored) {
                }
            });

            // Основной цикл чтения байтов из виртуального UART
            int b;
            while ((b = in.read()) != -1) {
                cli.onByteReceived(b);
            }
        }
    }

    private static synchronized void renderStatus(Garland garland, CliParser cli) {
        if (cli == null) return;

        Step s = garland.getCurrentStep();

        String gLed = s.g() ? (ANSI_GREEN  + "● ON " + ANSI_RESET) : (ANSI_GRAY + "○ OFF" + ANSI_RESET);
        String yLed = s.y() ? (ANSI_YELLOW + "● ON " + ANSI_RESET) : (ANSI_GRAY + "○ OFF" + ANSI_RESET);
        String rLed = s.r() ? (ANSI_RED    + "● ON " + ANSI_RESET) : (ANSI_GRAY + "○ OFF" + ANSI_RESET);

        String drvMode = cli.isInterruptsEnabled() ? "IT (Прерывания)" : "POLL (Опрос)   ";

        System.out.printf("\r[Режим %d/%d | Шаг %d/%d]  PD13(G): %s | PD14(Y): %s | PD15(R): %s  [UART: %s]   ",
                garland.getCurrentModeNum(),
                garland.getModesCount(),
                garland.getCurrentStepIndex(),
                garland.getCurrentModeStepsCount(),
                gLed, yLed, rLed, drvMode
        );
        System.out.flush();
    }
}