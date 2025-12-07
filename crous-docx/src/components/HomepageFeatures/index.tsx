import type {ReactNode} from 'react';
import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';

type FeatureItem = {
  title: string;
  icon: string;
  description: ReactNode;
};

const FeatureList: FeatureItem[] = [
  {
    title: 'Lightning Fast',
    icon: '⚡',
    description: (
      <>
        Optimized with C extensions for maximum performance. Serialize and deserialize
        large datasets in milliseconds, not seconds.
      </>
    ),
  },
  {
    title: 'Compact Binary Format',
    icon: '📦',
    description: (
      <>
        Produces 30-50% smaller output than JSON. Save bandwidth and storage space
        while maintaining full type information.
      </>
    ),
  },
  {
    title: 'Type Preserving',
    icon: '🎯',
    description: (
      <>
        Automatically preserves Python types. Distinguish between int and float,
        bytes and strings, and custom objects.
      </>
    ),
  },
  {
    title: 'Easy Integration',
    icon: '🔌',
    description: (
      <>
        Simple API similar to the <code>json</code> module. Drop-in replacement with
        familiar <code>dumps()</code> and <code>loads()</code> functions.
      </>
    ),
  },
  {
    title: 'Extensible',
    icon: '🔧',
    description: (
      <>
        Support for custom serializers and deserializers. Handle any Python object
        type with your custom logic.
      </>
    ),
  },
  {
    title: 'Production Ready',
    icon: '✅',
    description: (
      <>
        Thoroughly tested with comprehensive error handling. Reliable and stable for
        production environments.
      </>
    ),
  },
];

function Feature({title, icon, description}: FeatureItem) {
  return (
    <div className={clsx('col col--4', styles.featureCol)}>
      <div className={styles.featureCard}>
        <div className={styles.featureIcon}>{icon}</div>
        <Heading as="h3" className={styles.featureTitle}>{title}</Heading>
        <p className={styles.featureDesc}>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures(): ReactNode {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className={styles.sectionHeader}>
          <Heading as="h2">Why Choose Crous?</Heading>
          <p>Built for performance, designed for simplicity</p>
        </div>
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
