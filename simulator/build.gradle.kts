plugins {
    application
    java
}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(21))
    }
}

application {
    mainClass.set("sim.SimulatorApp")
}

tasks.named<JavaExec>("run") {
    standardInput = System.`in`
}