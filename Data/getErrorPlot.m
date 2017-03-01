function getErrorPlot(run1,run2,run3,run4,run5)
        x = [5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100 150 200 250 300 350 400 450 500];
        %x = 0.05:0.05:1;
        
        figure
        hold on
        runs1 = run1./max(run1);
        runs2 = run2./max(run2);
        runs3 = run3./max(run3);
        runs4 = run4./max(run4);
        runs5 = run5./max(run5);
        
        plot(x,runs1,'DisplayName','Try #1');
        plot(x,runs2,'DisplayName','Try #2');
        plot(x,runs3,'DisplayName','Try #3');
        plot(x,runs4,'DisplayName','Try #4');
        plot(x,runs5,'DisplayName','Try #5');
        xlabel('Population Size (NP)');
        ylabel('Error');
        legend('show')
        
        hold off
        
        sum = run1 + run2 + run3 + run4 + run5;
        mean = sum./ 5;
        mean = mean./max(mean);
        [m i] = min(mean)
        figure
        plot(x,mean);
        xlabel('Population Size (NP)');
        ylabel('Error');
        
end