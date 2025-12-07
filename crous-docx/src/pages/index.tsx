import type {ReactNode} from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import Heading from '@theme/Heading';

import styles from './index.module.css';

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className={styles.animatedBackground}></div>
      <div className="container">
        <div className={styles.headerContent}>
          <Heading as="h1" className={clsx("hero__title", styles.titleAnimated)}>
            {siteConfig.title}
          </Heading>
          <p className={clsx("hero__subtitle", styles.subtitleAnimated)}>
            {siteConfig.tagline}
          </p>
          <p className={styles.description}>
            Lightning-fast binary serialization for Python. Type-preserving, compact, and extensible.
          </p>
          <div className={styles.buttons}>
            <Link
              className="button button--secondary button--lg"
              to="/">
              Get Started →
            </Link>
            <Link
              className="button button--outline button--secondary button--lg"
              to="/docs/guides/installation">
              Install Now
            </Link>
          </div>
        </div>
      </div>
    </header>
  );
}

function StatsSection() {
  return (
    <section className={styles.statsSection}>
      <div className="container">
        <div className={styles.statsGrid}>
          <div className={styles.statItem}>
            <div className={styles.statNumber}>30-50%</div>
            <div className={styles.statLabel}>Smaller than JSON</div>
          </div>
          <div className={styles.statItem}>
            <div className={styles.statNumber}>&lt;1ms</div>
            <div className={styles.statLabel}>Typical latency</div>
          </div>
          <div className={styles.statItem}>
            <div className={styles.statNumber}>∞</div>
            <div className={styles.statLabel}>Custom serializers</div>
          </div>
          <div className={styles.statItem}>
            <div className={styles.statNumber}>3.7+</div>
            <div className={styles.statLabel}>Python versions</div>
          </div>
        </div>
      </div>
    </section>
  );
}

export default function Home(): ReactNode {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title={`${siteConfig.title} - High-Performance Binary Serialization`}
      description="Crous: A powerful binary serialization library for Python with type preservation and compact output.">
      <HomepageHeader />
      <main>
        <StatsSection />
        <HomepageFeatures />
      </main>
    </Layout>
  );
}
